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