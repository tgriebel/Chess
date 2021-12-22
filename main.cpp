﻿#include <iostream>
#include <string>
#include <algorithm>
#include "chess.h"
#include "piece.h"
#include "chessGame.h"
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

std::string PrintSquare( const Chess& board, const int x, const int y ) {
	std::string squareFormat;
	const Piece* piece = board.GetPiece( x, y );
	const bool isBlack = ( x % 2 ) == ( y % 2 );
	if ( piece != nullptr ) {
		const pieceType_t type = piece->type;
		const teamCode_t team = piece->team;
		const int instance = piece->instance;
		if ( team == teamCode_t::BLACK ) {
			if ( isBlack ) {
				SetTextColor( 245 );
			} else {
				SetTextColor( 5 );
			}
		} else {
			if ( isBlack ) {
				SetTextColor( 252 );
			}
			else {
				SetTextColor( 12 );
			}
		}
		squareFormat += " ";
		squareFormat += GetPieceCode( type );
		squareFormat += '0' + instance;
		squareFormat += ( team == teamCode_t::WHITE ) ? " " : "'";
	} else {
		if ( isBlack ) {
			SetTextColor( 240 );
		}else {
			SetTextColor( 15 );
		}
		squareFormat += ( isBlack ? "    " : "    " );
	}
	return squareFormat;
}

std::string PrintTeamCapturres( const Chess& board, const teamCode_t team ) {
	int captureCount = 0;
	const Piece* captures[ Chess::TeamPieceCount ];
	board.GetTeamCaptures( team, captures, captureCount );
	std::string captureFormat;
	captureFormat = "    Captures: ";
	for ( int i = 0; i < captureCount; ++i ) {
		if ( captures == nullptr ) {
			break;
		}
		captureFormat += GetPieceCode( captures[ i ]->type );
		captureFormat += captures[ i ]->instance + '0';
		captureFormat += ( team == teamCode_t::BLACK ) ? "" : "\'";
		captureFormat += ", ";
	}
	return captureFormat;
}

void PrintBoard( const Chess& board, const bool printCaptures ) {
	std::cout << "   ";
	for ( int i = 0; i < BoardSize; ++i ) {
		std::cout << "  ";
		std::cout << char( i + 'a' );
		std::cout << "  ";
	}
	std::cout << "\n   +";
	for ( int i = 0; i < BoardSize; ++i ) {
		std::cout << "----+";
	}
	std::cout << "\n";
	for ( int j = 0; j < BoardSize; ++j ) {
		std::cout << ( BoardSize - j );
		SetTextColor( 15 );
		std::cout << "  |";
		for ( int i = 0; i < BoardSize; ++i ) {
			const bool isBlack = ( j % 2 ) == ( i % 2 );
			SetTextColor( 15 );
			std::cout << PrintSquare( board, i, j );
			SetTextColor( 15 );
			std::cout << "|";
		}
		if ( printCaptures ) {
			if ( j == 0 ) {
				std::cout << PrintTeamCapturres( board, teamCode_t::BLACK );
			} else if ( j == 7 ) {
				std::cout << PrintTeamCapturres( board, teamCode_t::WHITE );
			}
		}
		std::cout << "\n   +";
		for ( int i = 0; i < BoardSize; ++i ) {
			std::cout << "----+";
		}
		std::cout << "\n";
	};
}

void RunTestCommands( Chess& board, std::vector< std::string >& commands ) {
	int turnNum = 0;
	teamCode_t turnTeam = teamCode_t::WHITE;
	for ( auto it = commands.begin(); it != commands.end(); ++it ) {
		command_t cmd{};
		resultCode_t result = TranslateCommandString( board, turnTeam, *it, cmd );
		if ( result != RESULT_SUCCESS ) {
			std::cout << *it << "-> Inavlid String" << std::endl;
			continue;
		}
		if ( board.Execute( cmd ) == false ) {
			std::cout << *it << "-> Invalid Move" << std::endl;
			continue;
		}
		std::cout << turnNum  << ": " << *it << "-> Completed" << std::endl;
		turnTeam = ( turnTeam == teamCode_t::WHITE ) ? teamCode_t::BLACK : teamCode_t::WHITE;
		++turnNum;
		PrintBoard( board, true );
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

	while ( true ) {
		teamCode_t nextTeam;

		// Print Board
clear_screen:
		{
			ClearScreen();
			//std::wcout << L"♔";
			PrintBoard( board, true );
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
			char c_str[ 16 ];
			std::cin >> c_str;
			commandString = c_str;
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
			command_t cmd{};
			resultCode_t result = TranslateCommandString( board, turnTeam, commandString, cmd );
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

	gameConfig_t cfg;
#define TEST
#if defined( TEST )
	{
		std::vector< std::string > commands;
		LoadConfig( "tests/default_board.txt", cfg );
		LoadHistory( "tests/commands_basic.txt", commands );

		Chess board( cfg );
		RunTestCommands( board, commands );
	}
#else
	LoadConfig( "tests/enpassant.txt", cfg );
	RunCmdLineGameLoop( cfg );
#endif
}