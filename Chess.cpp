#include <iostream>
#include <string>
#include <algorithm>
#include "chess.h"
#include "piece.h"
#include "board.h"

static char GetPieceCode( const pieceType_t type ) {
	switch ( type ) {
		case pieceType_t::PAWN:		return 'p';
		case pieceType_t::ROOK:		return 'r';
		case pieceType_t::KNIGHT:	return 'n';
		case pieceType_t::BISHOP:	return 'b';
		case pieceType_t::KING:		return 'k';
		case pieceType_t::QUEEN:	return 'q';
	}
	return '?';
}

static pieceType_t GetPieceType( const char code ) {
	switch ( tolower( code ) ) {
		case 'p':	return pieceType_t::PAWN;
		case 'r':	return pieceType_t::ROOK;
		case 'n':	return pieceType_t::KNIGHT;
		case 'b':	return pieceType_t::BISHOP;
		case 'k':	return pieceType_t::KING;
		case 'q':	return pieceType_t::QUEEN;
	}
	return pieceType_t::NONE;
}

static int GetFileNum( const char file ) {
	switch ( tolower( file ) ) {
		case 'a':	return 0;
		case 'b':	return 1;
		case 'c':	return 2;
		case 'd':	return 3;
		case 'e':	return 4;
		case 'f':	return 5;
		case 'g':	return 6;
		case 'h':	return 7;
	}
	return -1;
}

static char GetFile( const int fileNum ) {
	switch ( fileNum ) {
		case 0:		return 'a';
		case 1:		return 'b';
		case 2:		return 'c';
		case 3:		return 'd';
		case 4:		return 'e';
		case 5:		return 'f';
		case 6:		return 'g';
		case 7:		return 'h';
	}
	return '?';
}

static int GetRankNum( const char rank ) {
	switch ( tolower( rank ) ) {
		case '1':	return 7;
		case '2':	return 6;
		case '3':	return 5;
		case '4':	return 4;
		case '5':	return 3;
		case '6':	return 2;
		case '7':	return 1;
		case '8':	return 0;
	}
	return -1;
}

static char GetRank( const int rankNum ) {
	switch ( rankNum ) {
		case 0:		return '8';
		case 1:		return '7';
		case 2:		return '6';
		case 3:		return '5';
		case 4:		return '4';
		case 5:		return '3';
		case 6:		return '2';
		case 7:		return '1';
	}
	return '?';
}

command_t TranslateCommandString( const ChessBoard& board, const char* commandString ) {
	command_t cmd;
	cmd.pieceType = GetPieceType( commandString[ 0 ] );
	return cmd;
}

std::string PrintSquare( const ChessBoard& board, const int x, const int y ) {
	std::string squareFormat;
	const Piece* piece = board.GetPiece( x, y );
	if ( piece != nullptr ) {
		const pieceType_t type = piece->type;
		const teamCode_t team = piece->team;
		const int instance = piece->instance;
		squareFormat += " ";
		squareFormat += GetPieceCode( type );
		squareFormat += '0' + instance;
		squareFormat += ( team == teamCode_t::WHITE ) ? " " : "'";
	} else {
		const bool isBlack = ( x % 2 ) == ( y % 2 );
		squareFormat += ( isBlack ? "####" : "    " );
	}
	return squareFormat;
}

int main()
{
	ChessBoard board;
	int turnNum = 0;
	teamCode_t turnTeam = teamCode_t::WHITE;
	while ( true ) {
		teamCode_t nextTeam;

		// Print Board
		{
			system( "CLS" );
			std::cout << "+";
			for ( int i = 0; i < BoardSize; ++i ) {
				std::cout << "----+";
			}
			std::cout << std::endl;
			for ( int j = 0; j < BoardSize; ++j ) {
				std::cout << "|";
				for ( int i = 0; i < BoardSize; ++i ) {
					const bool isBlack = ( j % 2 ) == ( i % 2 );
					std::cout << PrintSquare( board, i, j ) << "|";
				}
				std::cout << std::endl << "+";
				for ( int i = 0; i < BoardSize; ++i ) {
					std::cout << "----+";
				}
				std::cout << std::endl;
			}
		}

		read_input:
		// Input
		std::string commandString;
		{
			if ( turnTeam == teamCode_t::WHITE ) {
				std::cout << turnNum << ": White>>";
				nextTeam = teamCode_t::BLACK;
			}
			else if ( turnTeam == teamCode_t::BLACK ) {
				std::cout << turnNum << ": Black>>";
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
				break;
			}
			if ( commandString.size() != 3 ) {
				std::cout << "Invalid action" << std::endl;
				goto read_input;
			}
			command_t cmd;
			cmd.pieceType = GetPieceType( commandString[ 0 ] );
			cmd.x = GetFileNum( commandString[ 1 ] );
			cmd.y = GetRankNum( commandString[ 2 ] );
			cmd.team = turnTeam;
			cmd.instance = 0;
		//	board.Execute( cmd );
		}
		turnTeam = nextTeam;
	}
}