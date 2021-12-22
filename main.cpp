#include <iostream>
#include <string>
#include <algorithm>
#include "chess.h"
#include "piece.h"
#include "game.h"
#include "tests.h"
#include "commands.h"

#include <windows.h>

void SetTextColor( const int color ) {
	HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
	SetConsoleTextAttribute( hConsole, color );
}

void SetWindowTitle( const wchar_t* title ) {
	SetConsoleTitle( title );
}

void ClearScreen() {
	system( "CLS" );
}

void PrintBoard( const Chess& board, std::vector< moveAction_t >* actions, const bool printCaptures ) {
	std::cout << "   ";
	for ( int i = 0; i < BoardSize; ++i ) {
		std::cout << "  ";
		std::cout << char( i + 'a' );
		std::cout << "  ";
	}
	std::cout << "\n   ";
	std::cout << "+----+----+----+----+----+----+----+----+";
	std::cout << "\n";
	for ( int j = 0; j < BoardSize; ++j ) {
		std::cout << char( BoardSize - j + '0' );
		SetTextColor( 15 );
		std::cout << "  |";
		for ( int i = 0; i < BoardSize; ++i ) {
			const bool isBlack = ( j % 2 ) == ( i % 2 );
			SetTextColor( 15 );
			const Piece* piece = board.GetPiece( i, j );
			if ( piece != nullptr ) {
				const teamCode_t team = piece->team;
				if ( team == teamCode_t::BLACK ) {
					if ( isBlack ) {
						SetTextColor( 245 );
					} else {
						SetTextColor( 5 );
					}
				} else if ( team == teamCode_t::WHITE ) {
					if ( isBlack ) {
						SetTextColor( 252 );
					} else {
						SetTextColor( 12 );
					}
				}
			} else {
				if ( isBlack ) {
					SetTextColor( 240 );
				} else {
					SetTextColor( 15 );
				}
			}
			std::string square = SquareToString( board, i, j );
			if ( actions != nullptr ) {
				for ( size_t action = 0; action < actions->size(); ++action ) {
					if ( ( (*actions)[ action ].x == i ) && ( ( *actions )[ action ].y == j ) ) {
						if ( isBlack ) {
							SetTextColor( 242 );
						} else {
							SetTextColor( 2 );
						}
						square = "****";
					}
				}
			}
			std::cout << square;
			SetTextColor( 15 );
			std::cout << "|";
		}
		if ( printCaptures ) {
			if ( j == 0 ) {
				std::cout << TeamCaptureString( board, teamCode_t::BLACK );
			} else if ( j == 7 ) {
				std::cout << TeamCaptureString( board, teamCode_t::WHITE );
			}
		}
		std::cout << "\n   ";
		std::cout << "+----+----+----+----+----+----+----+----+";
		std::cout << "\n";
	};
}

void RunTestCommands( Chess& board, std::vector< std::string >& commands ) {
	int turnNum = 0;
	teamCode_t turnTeam = teamCode_t::WHITE;
	for ( auto it = commands.begin(); it != commands.end(); ++it ) {
		command_t cmd{};
		resultCode_t result = TranslateActionCommand( board, turnTeam, *it, cmd );
		if ( result != RESULT_SUCCESS ) {
			std::cout << *it << "-> Inavlid String" << std::endl;
			continue;
		}
		if ( board.Execute( cmd ) == resultCode_t::RESULT_GAME_INVALID_MOVE ) {
			std::cout << *it << "-> Invalid Move" << std::endl;
			continue;
		}
		std::cout << turnNum  << ": " << *it << "-> Completed" << std::endl;
		turnTeam = ( turnTeam == teamCode_t::WHITE ) ? teamCode_t::BLACK : teamCode_t::WHITE;
		++turnNum;
		PrintBoard( board, nullptr, true );
	}
//	ClearScreen();
//	PrintBoard( board, true );
}

void ProcessEvent( callbackEvent_t& event ) {
	std::cout << "Enter Pawn Promotion\n";
	std::cout << "(Q)ueen, K(N)ight, (B)ishop, (R)ook: ";

	std::string choice;
	std::cin >> choice;
	event.promotionType = GetPieceType( choice[ 0 ] );
}

void RunCmdLineGameLoop( gameConfig_t& cfg ) {
reset_game:
	int turnNum = 0;
	teamCode_t turnTeam = teamCode_t::WHITE;
	teamCode_t winner = teamCode_t::NONE;
	Chess board( cfg );
	board.SetEventCallback( &ProcessEvent );

	std::vector< moveAction_t > actions;

	while ( true ) {
		teamCode_t nextTeam;
		// Print Board
clear_screen:
		{
			ClearScreen();
			//std::wcout << L"♔";
			PrintBoard( board, &actions, true );
		}

		if ( winner != teamCode_t::NONE ) {
			goto exit_program;
		}

read_input:
		// Input
		std::string commandString;
		{
			if ( turnTeam == teamCode_t::WHITE ) {
				std::cout << turnNum << ": Red>>";
				nextTeam = teamCode_t::BLACK;
			}
			else if ( turnTeam == teamCode_t::BLACK ) {
				std::cout << turnNum << ": Purple>>";
				nextTeam = teamCode_t::WHITE;
			}
			std::getline( std::cin, commandString );
			std::transform( commandString.begin(), commandString.end(), commandString.begin(), []( unsigned char c ) { return std::tolower( c ); } );
		}

		// Execute Move
		{
			if ( commandString == "exit" ) {
				goto exit_program;
			}
			if ( commandString == "clear" ) {
				goto clear_screen;
			}
			if ( commandString == "reset" ) {
				goto reset_game;
			}
			if ( commandString.substr( 0, 6 ) == "select" ) {
				actions.clear();
				if ( commandString.size() < 6 ) {
					std::cout << GetErrorMsg( RESULT_INPUT_INVALID_COMMAND ) << std::endl;
					goto read_input;
				}
				std::string args = commandString.substr( 7, 9 );
				if ( ( args.size() != 2 ) && ( args.size() != 3 ) ) {
					std::cout << GetErrorMsg( RESULT_INPUT_INVALID_COMMAND ) << std::endl;
					goto read_input;
				}
				const pieceType_t pieceType = GetPieceType( args[ 0 ] );
				const int instance = args[ 1 ] - '0';
				const teamCode_t team = ( ( args.size() == 3 ) && ( args[ 2 ] == '\'' ) ) ? teamCode_t::BLACK : teamCode_t::WHITE;
				const pieceHandle_t hdl = board.FindPiece( team, pieceType, instance );
				const Piece* piece = board.GetPiece( hdl );
				if ( piece != nullptr ) {
					piece->EnumerateActions( actions );
					goto clear_screen;
				} else {
					std::cout << GetErrorMsg( RESULT_INPUT_INVALID_COMMAND ) << std::endl;
					goto read_input;
				}
			}
			command_t cmd{};
			resultCode_t result = TranslateActionCommand( board, turnTeam, commandString, cmd );
			if ( result != RESULT_SUCCESS ) {
				std::cout << GetErrorMsg( result ) << std::endl;
				goto read_input;
			}

			result = board.Execute( cmd );

			if ( result == RESULT_GAME_COMPLETE ) {
				winner = board.GetWinner();
			} else if ( result != RESULT_SUCCESS ) {
				std::cout << GetErrorMsg( RESULT_INPUT_INVALID_MOVE ) << std::endl;
				goto read_input;
			}
		}
		turnTeam = nextTeam;
	}
exit_program:
	if ( winner != teamCode_t::NONE ) {
		if ( winner == teamCode_t::WHITE ) {
			std::cout << "Red wins!" << std::endl;
		} else {
			std::cout << "Purple wins!" << std::endl;
		}
	}
	std::cout << "Game Complete" << std::endl;
}

int main()
{
	SetWindowTitle( L"Chess by Thomas Griebel" );

	//HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
	//// you can loop k higher to see more color choices
	//for ( int k = 1; k < 255; k++ )
	//{
	//	// pick the colorattribute k you want
	//	SetConsoleTextAttribute( hConsole, k );
	//	std::cout << k << " I want to be nice today!" << std::endl;
	//}

	gameConfig_t cfg;
//#define TEST
#if defined( TEST )
	{
		std::vector< std::string > commands;
		LoadConfig( "tests/default_board.txt", cfg );
		LoadHistory( "tests/commands_basic.txt", commands );

		Chess board( cfg );
		RunTestCommands( board, commands );
	}
#else
	LoadConfig( "tests/no_pawn.txt", cfg );
	RunCmdLineGameLoop( cfg );
#endif
}