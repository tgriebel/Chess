#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "common.h"
#include "piece.h"
#include "chess.h"
#include "commands.h"

using namespace std;

#define BP { teamCode_t::BLACK, pieceType_t::PAWN, 0, false }
#define BR { teamCode_t::BLACK, pieceType_t::ROOK, 0, false }
#define BN { teamCode_t::BLACK, pieceType_t::KNIGHT, 0, false }
#define BB { teamCode_t::BLACK, pieceType_t::BISHOP, 0, false }
#define BQ { teamCode_t::BLACK, pieceType_t::QUEEN, 0, false }
#define BK { teamCode_t::BLACK, pieceType_t::KING, 0, false }
#define WP { teamCode_t::WHITE, pieceType_t::PAWN, 0, false }
#define WR { teamCode_t::WHITE, pieceType_t::ROOK, 0, false }
#define WN { teamCode_t::WHITE, pieceType_t::KNIGHT, 0, false }
#define WB { teamCode_t::WHITE, pieceType_t::BISHOP, 0, false }
#define WQ { teamCode_t::WHITE, pieceType_t::QUEEN, 0, false }
#define WK { teamCode_t::WHITE, pieceType_t::KING, 0, false }
#define CL { teamCode_t::NONE, pieceType_t::NONE, 0, false }

void GetDefaultConfig( gameConfig_t& defaultCfg ) {
	static constexpr pieceInfo_t DefaultCfg[ BoardSize ][ BoardSize ] = {
		{ BR, BN, BB, BQ, BK, BB, BN, BR },
		{ BP, BP, BP, BP, BP, BP, BP, BP },
		{ CL, CL, CL, CL, CL, CL, CL, CL },
		{ CL, CL, CL, CL, CL, CL, CL, CL },
		{ CL, CL, CL, CL, CL, CL, CL, CL },
		{ CL, CL, CL, CL, CL, CL, CL, CL },
		{ WP, WP, WP, WP, WP, WP, WP, WP },
		{ WR, WN, WB, WQ, WK, WB, WN, WR },
	};
	for ( int i = 0; i < BoardSize; ++i ) {
		for ( int j = 0; j < BoardSize; ++j ) {
			defaultCfg.board[ i ][ j ] = DefaultCfg[ i ][ j ];
		}
	}
}

std::string SquareToString( const Chess& board, const int x, const int y ) {
	std::string squareFormat;
	const pieceInfo_t info = board.GetInfo( x, y );
	const bool isBlack = ( x % 2 ) == ( y % 2 );
	if ( info.onBoard == true ) {
		const pieceType_t type = info.piece;
		const teamCode_t team = info.team;
		const int instance = info.instance;
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
	pieceInfo_t captures[ TeamPieceCount ];
	board.GetTeamCaptures( team, captures, captureCount );
	std::string captureFormat;
	captureFormat = "    Captures: ";
	for ( int i = 0; i < captureCount; ++i ) {
		if ( captures == nullptr ) {
			break;
		}
		captureFormat += GetPieceCode( captures[ i ].piece );
		captureFormat += captures[ i ].instance + '0';
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

static pieceInfo_t GetSquareConfig( const string& token ) {
    if ( token.size() != 2 ) {
        return CL;
    }
    pieceInfo_t cfg;  
    switch ( token[ 0 ] ) {
        case 'B': cfg.team = teamCode_t::BLACK; break;
        case 'W': cfg.team = teamCode_t::WHITE; break;
    }
    cfg.piece = GetPieceType( token[ 1 ] );
	cfg.onBoard = ( cfg.piece != pieceType_t::NONE );
	cfg.instance = 0;
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