#pragma once
#include <assert.h>
#include <string>
#include <vector>

static const int BoardSize		= 8;
static const int TeamCount		= 2;
static const int TeamPieceCount	= 16;
static const int PieceCount		= 32;
typedef int pieceHandle_t;
static const pieceHandle_t NoPiece = -1;
static const pieceHandle_t OffBoard = -2;

class Chess;

enum resultCode_t {
	ERROR_RANGE_START				= -101,
	RESULT_INPUT_INVALID_COMMAND	= ERROR_RANGE_START,
	RESULT_INPUT_INVALID_PIECE,
	RESULT_INPUT_INVALID_FILE,
	RESULT_INPUT_INVALID_RANK,
	RESULT_INPUT_INVALID_MOVE,
	RESULT_GAME_INVALID_PIECE,
	RESULT_GAME_INVALID_MOVE,
	RESULT_GAME_COMPLETE,
	ERROR_COUNT						= ( RESULT_GAME_COMPLETE - RESULT_INPUT_INVALID_COMMAND ) + 1,
	RESULT_SUCCESS					= 0,
};

static const char* ErrorMsgs[ ERROR_COUNT ] =
{
	"Invalid command",			// RESULT_INPUT_INVALID_COMMAND
	"Invalid piece",			// RESULT_INPUT_INVALID_PIECE
	"File out of range",		// RESULT_INPUT_INVALID_FILE
	"Rank out of range",		// RESULT_INPUT_INVALID_RANK
	"Invalid move command",		// RESULT_INPUT_INVALID_MOVE
	"Invalid piece selected",	// RESULT_GAME_INVALID_PIECE
	"Invalid move action",		// RESULT_GAME_INVALID_MOVE
	"Game Complete",			// RESULT_GAME_COMPLETE
};

inline const char* GetErrorMsg( const resultCode_t code ) {
	const int index = code - ERROR_RANGE_START;
	return ErrorMsgs[ index ];
}

enum class pieceType_t : int {
	NONE = -1,
	PAWN = 0,
	ROOK,
	KNIGHT,
	BISHOP,
	KING,
	QUEEN,
	COUNT,
};

enum class teamCode_t : int {
	NONE = -1,
	WHITE = 0,
	BLACK = 1,
	COUNT
};

enum class moveType_t : int {
	NONE = -1,

	PAWN_T = 0,
	PAWN_T2X,
	PAWN_KILL_L,
	PAWN_KILL_R,

	KNIGHT_T1L2,
	KNIGHT_T2L1,
	KNIGHT_T1R2,
	KNIGHT_T2R1,
	KNIGHT_B1R2,
	KNIGHT_B2R1,
	KNIGHT_B2L1,
	KNIGHT_B1L2,

	ROOK_L,
	ROOK_R,
	ROOK_T,
	ROOK_B,

	BISHOP_TL,
	BISHOP_TR,
	BISHOP_BR,
	BISHOP_BL,

	KING_TL,
	KING_T,
	KING_TR,
	KING_R,
	KING_BR,
	KING_B,
	KING_BL,
	KING_L,
	KING_CASTLE_L,
	KING_CASTLE_R,

	QUEEN_TL,
	QUEEN_T,
	QUEEN_TR,
	QUEEN_R,
	QUEEN_BR,
	QUEEN_B,
	QUEEN_BL,
	QUEEN_L,

	PAWN_ACTIONS	= ( PAWN_KILL_R - PAWN_T ) + 1,
	KNIGHT_ACTIONS	= ( KNIGHT_B1L2 - KNIGHT_T1L2 ) + 1,
	ROOK_ACTIONS	= ( ROOK_B - ROOK_L ) + 1,
	BISHOP_ACTIONS	= ( BISHOP_BL - BISHOP_TL ) + 1,
	KING_ACTIONS	= ( KING_CASTLE_R - KING_TL ) + 1,
	QUEEN_ACTIONS	= ( QUEEN_L - QUEEN_TL ) + 1,
	MOVE_COUNT		= ( PAWN_ACTIONS + KNIGHT_ACTIONS + ROOK_ACTIONS + BISHOP_ACTIONS + KING_ACTIONS + QUEEN_ACTIONS ),
};

enum callbackEventType_t {
	PAWN_PROMOTION
};

struct callbackEvent_t {
	callbackEventType_t	type;
	pieceType_t			promotionType;
};

struct moveAction_t {
	moveAction_t() {
		x = 0;
		y = 0;
		maxSteps = 0;
		type = moveType_t::NONE;
	}
	moveAction_t( const int x, int const y, const moveType_t type, const int maxSteps ) :
		x( x ), y( y ), type( type ), maxSteps( maxSteps ) {}
	int				x;
	int				y;
	int				maxSteps;
	moveType_t		type;
};

struct command_t {
	int				x;
	int				y;
	pieceType_t		pieceType;
	int				instance;
	teamCode_t		team;
};

struct team_t {
	team_t() {
		for ( int i = 0; i < TeamPieceCount; ++i ) {
			pieces[ i ] = 0;
			captured[ i ] = 0;
		}
		for ( int i = 0; i < (int)pieceType_t::COUNT; ++i ) {
			typeCounts[ i ] = 0;
			captureTypeCounts[ i ] = 0;
		}
		livingCount = 0;
		capturedCount = 0;
	}

	pieceHandle_t	pieces[ TeamPieceCount ];
	pieceHandle_t	captured[ TeamPieceCount ];
	int				livingCount;
	int				capturedCount;
	int				typeCounts[ (int)pieceType_t::COUNT ];
	int				captureTypeCounts[ (int)pieceType_t::COUNT ];
};

struct pieceInfo_t {
	teamCode_t	team;
	pieceType_t	piece;
	int			instance;
	bool		onBoard;
};

struct gameConfig_t {
	gameConfig_t() {};

	gameConfig_t( const gameConfig_t& config ) {
		for ( int i = 0; i < BoardSize; ++i ) {
			for ( int j = 0; j < BoardSize; ++j ) {
				board[ i ][ j ] = config.board[ i ][ j ];
			}
		}
	}

	gameConfig_t& operator=( const gameConfig_t& config ) {
		if ( this == &config ) {
			return *this;
		}
		for ( int i = 0; i < BoardSize; ++i ) {
			for ( int j = 0; j < BoardSize; ++j ) {
				board[ i ][ j ] = config.board[ i ][ j ];
			}
		}		
		return *this;
	}

	pieceInfo_t board[ BoardSize ][ BoardSize ];
};

typedef void ( *callback_t )( callbackEvent_t& );

void GetDefaultConfig( gameConfig_t& defaultCfg );
std::string SquareToString( const Chess& board, const int x, const int y );
std::string TeamCaptureString( const Chess& board, const teamCode_t team );
std::string BoardToString( const Chess& board, const bool printCaptures );
void LoadConfig( const std::string& fileName, gameConfig_t& config );
void LoadHistory( const std::string& fileName, std::vector< std::string >& commands );