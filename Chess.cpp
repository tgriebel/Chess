#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "chess.h"
#include "piece.h"
#include "chessGame.h"
#include "commands.h"

using namespace std;

//BR, BN, BB, BQ, BK, BB, BN, BR
//BP, BP, BP, BP, BP, BP, BP, BP
//CL, CL, CL, CL, CL, CL, CL, CL
//CL, CL, CL, CL, CL, CL, CL, CL
//CL, CL, CL, CL, CL, CL, CL, CL
//CL, CL, CL, CL, CL, CL, CL, CL
//WP, WP, WP, WP, WP, WP, WP, WP
//WR, CL, CL, CL, WK, WB, WN, WR

std::string SquareToString( const Chess& board, const int x, const int y ) {
	std::string squareFormat;
	const Piece* piece = board.GetPiece( x, y );
	const bool isBlack = ( x % 2 ) == ( y % 2 );
	if ( piece != nullptr ) {
		const pieceType_t type = piece->type;
		const teamCode_t team = piece->team;
		const int instance = piece->instance;
		squareFormat += " ";
		squareFormat += GetPieceCode( type );
		squareFormat += '0' + instance;
		squareFormat += ( team == teamCode_t::WHITE ) ? " " : "'";
	} else {
		squareFormat += ( isBlack ? "    " : "    " );
	}
	return squareFormat;
}

std::string TeamCaptureString( const Chess& board, const teamCode_t team ) {
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

std::string BoardToString( const Chess& board, const bool printCaptures ) {
	std::string boardFormat;
	boardFormat = "   ";
	for ( int i = 0; i < BoardSize; ++i ) {
		boardFormat += "  ";
		boardFormat += char( i + 'a' );
		boardFormat += "  ";
	}
	boardFormat += "\n   ";
	boardFormat += "+----+----+----+----+----+----+----+----+";
	boardFormat += "\n";
	for ( int j = 0; j < BoardSize; ++j ) {
		boardFormat += char( BoardSize - j + '0' );
		boardFormat += "  |";
		for ( int i = 0; i < BoardSize; ++i ) {
			boardFormat += SquareToString( board, i, j );
			boardFormat += "|";
		}
		if ( printCaptures ) {
			if ( j == 0 ) {
				boardFormat += TeamCaptureString( board, teamCode_t::BLACK );
			} else if ( j == 7 ) {
				boardFormat += TeamCaptureString( board, teamCode_t::WHITE );
			}
		}
		boardFormat += "\n   ";
		boardFormat += "+----+----+----+----+----+----+----+----+";
		boardFormat += "\n";
	};
	return boardFormat;
}

static squareCfg_t GetSquareConfig( const string& token ) {
    if ( token.size() != 2 ) {
        return CL;
    }
    squareCfg_t cfg;   
    switch ( token[ 0 ] ) {
        case 'B': cfg.team = teamCode_t::BLACK; break;
        case 'W': cfg.team = teamCode_t::WHITE; break;
    }
    cfg.piece = GetPieceType( token[ 1 ] );
    return cfg;
}

void LoadConfig( const std::string& fileName, gameConfig_t& config ) {
    string line;
    ifstream configFile( fileName );
    int row = 0;
    int col = 0;
    if ( configFile.is_open() ) {
        while ( getline( configFile, line ) ) {
            stringstream lineStream( line );
            while ( lineStream.eof() == false ) {
                string token;
                lineStream >> token;
                string delimiter = ",";
                token = token.substr( 0, token.find( delimiter ) );
                config.board[ row ][ col ] = GetSquareConfig( token );
                ++col;
            }
            col = 0;
            ++row;
        }
        configFile.close();
    }
}

void LoadHistory( const std::string& fileName, std::vector< std::string >& commands ) {
    string line;
    ifstream configFile( fileName );
    if ( configFile.is_open() ) {
        while ( getline( configFile, line ) ) {
            commands.push_back( line );
        }
        configFile.close();
    }
}