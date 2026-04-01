#include "Chess.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <functional>
#include <sstream>
#include <cstdint>


// ============================================================
// Expected outcomes
// ============================================================

enum class ExpectedOutcome
{
	GAME_IN_PROGRESS,
	WHITE_WINS,
	BLACK_WINS,
	STALEMATE,
};

static const char* OutcomeToString( const ExpectedOutcome outcome )
{
	switch ( outcome )
	{
	case ExpectedOutcome::GAME_IN_PROGRESS:	return "GAME_IN_PROGRESS";
	case ExpectedOutcome::WHITE_WINS:		return "WHITE_WINS";
	case ExpectedOutcome::BLACK_WINS:		return "BLACK_WINS";
	case ExpectedOutcome::STALEMATE:		return "STALEMATE";
	}
	return "UNKNOWN";
}

static ExpectedOutcome WinnerToOutcome( const teamCode_t winner )
{
	switch ( winner )
	{
	case teamCode_t::WHITE:	return ExpectedOutcome::WHITE_WINS;
	case teamCode_t::BLACK:	return ExpectedOutcome::BLACK_WINS;
	default:				return ExpectedOutcome::GAME_IN_PROGRESS;
	}
}


// ============================================================
// Per-move expectations
// ============================================================

struct MoveExpectation
{
	int32_t			moveIndex;
	resultCode_t	expectedResult;
};

struct PieceExpectation
{
	pieceType_t		type;
	teamCode_t		team;
	int32_t			instance;
	int32_t			expectedX;		// -1 = captured / off board
	int32_t			expectedY;		// -1 = captured / off board
};


// ============================================================
// Test case definition
// ============================================================

struct TestCase
{
	const char*						name;
	const char*						description;
	const char*						boardFile;		// nullptr = use default board
	const char*						commandsFile;	// nullptr = use inlineCommands
	std::vector< std::string >		inlineCommands;
	ExpectedOutcome					expectedOutcome;
	std::vector< MoveExpectation >	moveExpectations;
	std::vector< PieceExpectation >	pieceExpectations;
};


// ============================================================
// Test result
// ============================================================

struct TestResult
{
	const char*		name;
	const char*		description;
	bool			passed;
	std::string		details;
	int32_t			movesExecuted;
	int32_t			totalMoves;
};


// ============================================================
// Test registry
// ============================================================

static std::vector< TestCase >& GetTestRegistry()
{
	static std::vector< TestCase > tests;
	return tests;
}

static int32_t RegisterTest( TestCase tc )
{
	GetTestRegistry().push_back( std::move( tc ) );
	return 0;
}

#define REGISTER_TEST( varName ) \
	static int32_t _reg_##varName = RegisterTest( varName )


// ============================================================
// Logger
// ============================================================

class TestLogger
{
public:
	TestLogger( const char* logPath )
	{
		logFile.open( logPath, std::ios::out | std::ios::trunc );
	}

	~TestLogger()
	{
		if ( logFile.is_open() )
		{
			logFile.close();
		}
	}

	void Write( const std::string& line )
	{
		std::cout << line << "\n";
		if ( logFile.is_open() )
		{
			logFile << line << "\n";
		}
	}

	void Flush()
	{
		std::cout.flush();
		if ( logFile.is_open() )
		{
			logFile.flush();
		}
	}

private:
	std::ofstream logFile;
};


// ============================================================
// Promotion callback (auto-queen for tests)
// ============================================================

static void AutoPromoteQueen( callbackEvent_t& event )
{
	event.promotionType = pieceType_t::QUEEN;
}


// ============================================================
// Test runner
// ============================================================

static TestResult RunSingleTest( const TestCase& tc )
{
	TestResult result;
	result.name = tc.name;
	result.description = tc.description;
	result.passed = true;
	result.movesExecuted = 0;
	result.totalMoves = 0;

	// Load board
	gameConfig_t cfg;
	if ( tc.boardFile != nullptr )
	{
		LoadConfig( tc.boardFile, cfg );
	}
	else
	{
		GetDefaultConfig( cfg );
	}

	// Load commands
	std::vector< std::string > commands;
	if ( tc.commandsFile != nullptr )
	{
		LoadHistory( tc.commandsFile, commands );
	}
	else
	{
		commands = tc.inlineCommands;
	}
	result.totalMoves = static_cast<int32_t>( commands.size() );

	// Create engine
	ChessEngine engine( cfg );
	engine.SetEventCallback( &AutoPromoteQueen );

	// Execute commands
	teamCode_t turnTeam = teamCode_t::WHITE;

	for ( int32_t i = 0; i < static_cast<int32_t>( commands.size() ); ++i )
	{
		command_t cmd{};
		resultCode_t translateResult = TranslateActionCommand( engine, turnTeam, commands[ i ], cmd );

		resultCode_t moveResult;
		if ( translateResult != RESULT_SUCCESS )
		{
			moveResult = translateResult;
		}
		else
		{
			moveResult = engine.Execute( cmd );
		}

		// Check per-move expectations
		for ( const auto& me : tc.moveExpectations )
		{
			if ( me.moveIndex == i )
			{
				if ( moveResult != me.expectedResult )
				{
					result.passed = false;
					result.details += "  Move " + std::to_string( i ) + " ('" + commands[ i ] + "'): ";
					result.details += "expected " + std::string( GetErrorMsg( me.expectedResult ) );
					result.details += ", got ";
					if ( moveResult == RESULT_SUCCESS )
					{
						result.details += "SUCCESS";
					}
					else
					{
						result.details += GetErrorMsg( moveResult );
					}
					result.details += "\n";
				}
			}
		}

		// Advance turn on success
		if ( moveResult == RESULT_SUCCESS || moveResult == RESULT_GAME_COMPLETE )
		{
			turnTeam = ( turnTeam == teamCode_t::WHITE ) ? teamCode_t::BLACK : teamCode_t::WHITE;
			++result.movesExecuted;
		}
	}

	// Check final outcome
	const ExpectedOutcome actualOutcome = WinnerToOutcome( engine.GetWinner() );
	if ( actualOutcome != tc.expectedOutcome )
	{
		result.passed = false;
		result.details += "  Outcome: expected ";
		result.details += OutcomeToString( tc.expectedOutcome );
		result.details += ", got ";
		result.details += OutcomeToString( actualOutcome );
		result.details += "\n";
	}

	// Check piece positions
	for ( const auto& pe : tc.pieceExpectations )
	{
		const pieceHandle_t hdl = engine.FindPiece( pe.team, pe.type, pe.instance );

		if ( pe.expectedX == -1 && pe.expectedY == -1 )
		{
			// Piece should be captured
			if ( hdl != NoPiece )
			{
				int32_t ax, ay;
				if ( engine.GetLocation( hdl, ax, ay ) && ax >= 0 && ay >= 0 )
				{
					result.passed = false;
					result.details += "  Piece (";
					result.details += ( pe.team == teamCode_t::WHITE ) ? "W" : "B";
					result.details += GetPieceCode( pe.type );
					result.details += std::to_string( pe.instance );
					result.details += "): expected captured, but found at ";
					result.details += GetFile( ax );
					result.details += GetRank( ay );
					result.details += "\n";
				}
			}
		}
		else
		{
			if ( hdl == NoPiece )
			{
				result.passed = false;
				result.details += "  Piece (";
				result.details += ( pe.team == teamCode_t::WHITE ) ? "W" : "B";
				result.details += GetPieceCode( pe.type );
				result.details += std::to_string( pe.instance );
				result.details += "): expected at ";
				result.details += GetFile( pe.expectedX );
				result.details += GetRank( pe.expectedY );
				result.details += ", but piece not found\n";
			}
			else
			{
				int32_t ax, ay;
				engine.GetLocation( hdl, ax, ay );
				if ( ax != pe.expectedX || ay != pe.expectedY )
				{
					result.passed = false;
					result.details += "  Piece (";
					result.details += ( pe.team == teamCode_t::WHITE ) ? "W" : "B";
					result.details += GetPieceCode( pe.type );
					result.details += std::to_string( pe.instance );
					result.details += "): expected at ";
					result.details += GetFile( pe.expectedX );
					result.details += GetRank( pe.expectedY );
					result.details += ", got ";
					result.details += GetFile( ax );
					result.details += GetRank( ay );
					result.details += "\n";
				}
			}
		}
	}

	return result;
}


// ============================================================
// Test case definitions
// ============================================================

// --- Opening games (default board) ---

static TestCase TestFoolsMate =
{
	"Fools Mate",
	"Shortest possible checkmate: 1.f3 e5 2.g4 Qh4#",
	"tests/default_board.txt",
	"tests/fools_mate.txt",
	{},
	ExpectedOutcome::BLACK_WINS,
	{},
	{}
};
REGISTER_TEST( TestFoolsMate );

static TestCase TestScholarsMate =
{
	"Scholars Mate",
	"Classic 4-move checkmate: 1.e4 e5 2.Bc4 Nc6 3.Qh5 Nf6 4.Qxf7#",
	"tests/default_board.txt",
	"tests/scholars_mate.txt",
	{},
	ExpectedOutcome::WHITE_WINS,
	{},
	{
		{ pieceType_t::PAWN, teamCode_t::BLACK, 5, -1, -1 },	// f7 pawn captured
	}
};
REGISTER_TEST( TestScholarsMate );

static TestCase TestItalianOpening =
{
	"Italian Opening",
	"Italian Game with both sides castling kingside",
	"tests/default_board.txt",
	"tests/italian_opening.txt",
	{},
	ExpectedOutcome::GAME_IN_PROGRESS,
	{},
	{
		// White castled: king on g1, rook on f1
		{ pieceType_t::KING, teamCode_t::WHITE, 0, 6, 7 },
		// Black castled: king on g8, rook on f8
		{ pieceType_t::KING, teamCode_t::BLACK, 0, 6, 0 },
	}
};
REGISTER_TEST( TestItalianOpening );

static TestCase TestQueensGambit =
{
	"Queens Gambit Declined",
	"Queen's Gambit Declined: 1.d4 d5 2.c4 e6 3.Nc3 Nf6 4.Bg5 Be7",
	"tests/default_board.txt",
	"tests/queens_gambit.txt",
	{},
	ExpectedOutcome::GAME_IN_PROGRESS,
	{},
	{}
};
REGISTER_TEST( TestQueensGambit );

static TestCase TestBasicCommands =
{
	"Basic Pawn Moves",
	"Sequence of pawn advances from default board",
	"tests/default_board.txt",
	"tests/commands_basic.txt",
	{},
	ExpectedOutcome::GAME_IN_PROGRESS,
	{},
	{}
};
REGISTER_TEST( TestBasicCommands );


// --- Tactical tests (custom boards) ---

static TestCase TestBackRankMate =
{
	"Back Rank Mate",
	"Rook delivers checkmate on 8th rank, king trapped by own pawns",
	"tests/back_rank_mate_board.txt",
	"tests/back_rank_mate_cmds.txt",
	{},
	ExpectedOutcome::WHITE_WINS,
	{
		{ 0, RESULT_GAME_COMPLETE },
	},
	{}
};
REGISTER_TEST( TestBackRankMate );

static TestCase TestPromotion =
{
	"Pawn Promotion",
	"White pawn on 7th rank promotes to queen",
	"tests/promotion_board.txt",
	"tests/promotion_cmds.txt",
	{},
	ExpectedOutcome::GAME_IN_PROGRESS,
	{
		{ 0, RESULT_SUCCESS },
	},
	{}
};
REGISTER_TEST( TestPromotion );

static TestCase TestEnPassant =
{
	"En Passant",
	"White double-pushes a2-a4, black captures en passant b4xa3",
	"tests/enpassant.txt",
	"tests/enpassant_cmds.txt",
	{},
	ExpectedOutcome::GAME_IN_PROGRESS,
	{
		{ 0, RESULT_SUCCESS },		// a2-a4 double push
		{ 1, RESULT_SUCCESS },		// bxa3 en passant
	},
	{
		{ pieceType_t::PAWN, teamCode_t::WHITE, 0, -1, -1 },	// white pawn captured via en passant
	}
};
REGISTER_TEST( TestEnPassant );

static TestCase TestKnightFork =
{
	"Knight Fork",
	"Knight forks king and queen at e7, captures queen after king flees",
	"tests/knight_fork_board.txt",
	"tests/knight_fork_cmds.txt",
	{},
	ExpectedOutcome::GAME_IN_PROGRESS,
	{
		{ 0, RESULT_SUCCESS },		// Ne7+ fork
		{ 1, RESULT_SUCCESS },		// Kf8
		{ 2, RESULT_SUCCESS },		// Nxc8
	},
	{
		{ pieceType_t::QUEEN, teamCode_t::BLACK, 0, -1, -1 },	// queen captured
		{ pieceType_t::KING, teamCode_t::BLACK, 0, 5, 0 },	// king on f8
	}
};
REGISTER_TEST( TestKnightFork );


// --- Bug-exposing tests ---

static TestCase TestStalemate =
{
	"Stalemate Detection",
	"Qb6 creates stalemate -- black has no legal moves but is not in check",
	"tests/stalemate_board.txt",
	"tests/stalemate_cmds.txt",
	{},
	// BUG: engine has no stalemate detection, so this should be STALEMATE
	// but the engine will likely report GAME_IN_PROGRESS or incorrectly declare a winner
	ExpectedOutcome::STALEMATE,
	{},
	{}
};
REGISTER_TEST( TestStalemate );

static TestCase TestPin =
{
	"Pinned Piece Cannot Move",
	"Black knight pinned to king by white bishop -- n0f6 should be illegal",
	"tests/pin_board.txt",
	"tests/pin_cmds.txt",
	{},
	ExpectedOutcome::GAME_IN_PROGRESS,
	{
		{ 0, RESULT_SUCCESS },						// White Kb1 (waiting move)
		{ 1, RESULT_GAME_INVALID_MOVE },			// Black Nf6 should be rejected (pinned)
	},
	{}
};
REGISTER_TEST( TestPin );

static TestCase TestCastleThroughCheck =
{
	"Castle Through Check",
	"Black rook controls f-file -- white cannot castle kingside through f1",
	"tests/castle_through_check_board.txt",
	"tests/castle_through_check_cmds.txt",
	{},
	ExpectedOutcome::GAME_IN_PROGRESS,
	{
		{ 0, RESULT_GAME_INVALID_MOVE },			// O-O should be rejected
	},
	{}
};
REGISTER_TEST( TestCastleThroughCheck );


// --- Inline command tests ---

static TestCase TestInvalidCommand =
{
	"Invalid Command Format",
	"Garbage input should be rejected",
	nullptr,
	nullptr,
	{ "zzzz", "p0e4" },
	ExpectedOutcome::GAME_IN_PROGRESS,
	{
		{ 0, RESULT_INPUT_INVALID_PIECE },			// 'z' is not a valid piece
		{ 1, RESULT_SUCCESS },						// valid move still works
	},
	{}
};
REGISTER_TEST( TestInvalidCommand );

static TestCase TestMoveOffBoard =
{
	"Move Off Board",
	"Attempting to move a piece to an invalid square",
	nullptr,
	nullptr,
	{ "p0a5" },
	ExpectedOutcome::GAME_IN_PROGRESS,
	{
		{ 0, RESULT_GAME_INVALID_MOVE },			// pawn can't jump 3 squares
	},
	{}
};
REGISTER_TEST( TestMoveOffBoard );

static TestCase TestFriendlyBlock =
{
	"Friendly Piece Blocking",
	"Rook cannot move through own pawn",
	nullptr,
	nullptr,
	{ "r0a2" },
	ExpectedOutcome::GAME_IN_PROGRESS,
	{
		{ 0, RESULT_GAME_INVALID_MOVE },			// rook blocked by pawn on a2
	},
	{}
};
REGISTER_TEST( TestFriendlyBlock );


// ============================================================
// main
// ============================================================

int main()
{
	TestLogger logger( "test_results.log" );

	const auto& tests = GetTestRegistry();

	// Header
	{
		auto now = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t( now );

		char timeBuf[ 64 ];
		struct tm tmBuf;
		localtime_s( &tmBuf, &t );
		std::strftime( timeBuf, sizeof( timeBuf ), "%Y-%m-%d %H:%M:%S", &tmBuf );

		logger.Write( "=============================================" );
		logger.Write( "  Chess Engine Test Results" );
		logger.Write( "  " + std::string( timeBuf ) );
		logger.Write( "=============================================" );
		logger.Write( "" );
	}

	// Run tests
	std::vector< TestResult > results;
	int32_t passed = 0;
	int32_t failed = 0;

	for ( const auto& tc : tests )
	{
		TestResult result = RunSingleTest( tc );
		results.push_back( result );
		if ( result.passed )
		{
			++passed;
		}
		else
		{
			++failed;
		}
	}

	// Results
	for ( const auto& r : results )
	{
		std::string status = r.passed ? "[PASS]" : "[FAIL]";
		logger.Write( status + " " + r.name );
		if ( r.description != nullptr )
		{
			logger.Write( "  " + std::string( r.description ) );
		}
		logger.Write( "  Moves: " + std::to_string( r.movesExecuted ) + "/" + std::to_string( r.totalMoves ) + " executed" );
		if ( !r.passed && !r.details.empty() )
		{
			logger.Write( r.details );
		}
		logger.Write( "" );
	}

	// Summary
	logger.Write( "=============================================" );
	logger.Write( "  Summary: " + std::to_string( passed ) + "/" + std::to_string( passed + failed ) + " PASSED" );
	if ( failed > 0 )
	{
		logger.Write( "  " + std::to_string( failed ) + " FAILED" );
	}
	logger.Write( "=============================================" );

	logger.Flush();

	return ( failed > 0 ) ? 1 : 0;
}
