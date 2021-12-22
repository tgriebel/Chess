#pragma once
#include <string>
#include <vector>
#include "chess.h"
#include "chessGame.h"
#include "piece.h"

static void ClearBoard( gameConfig_t& config ) {
	for ( int i = 0; i < BoardSize; ++i ) {
		for ( int j = 0; j < BoardSize; ++j ) {
			config.board[ i ][ j ] = CL;
		}
	}
}

static void PawnMovement_Init( gameConfig_t& config ) {
	ClearBoard( config );
	config.board[ 4 ][ 4 ] = WP;
	config.board[ 5 ][ 5 ] = BP;
}

static void RookMovement_Init( gameConfig_t& config ) {
	ClearBoard( config );
	config.board[ 4 ][ 4 ] = WR;
	config.board[ 5 ][ 5 ] = BR;
}

static void KnightMovement_Init( gameConfig_t& config ) {
	ClearBoard( config );
	config.board[ 4 ][ 4 ] = WK;
	config.board[ 5 ][ 5 ] = BK;
}

static void BishopMovement_Init( gameConfig_t& config ) {
	ClearBoard( config );
	config.board[ 4 ][ 4 ] = WB;
	config.board[ 5 ][ 5 ] = BB;
}

static void QueenMovement_Init( gameConfig_t& config ) {
	ClearBoard( config );
	config.board[ 4 ][ 4 ] = WQ;
	config.board[ 5 ][ 5 ] = BQ;
}

static void KingMovement_Init( gameConfig_t& config ) {
	ClearBoard( config );
	config.board[ 4 ][ 4 ] = WK;
	config.board[ 5 ][ 5 ] = BK;
}

static void WhiteCastle_Init( gameConfig_t& config ) {
	config.board[ 7 ][ 1 ] = CL;
	config.board[ 7 ][ 2 ] = CL;
	config.board[ 7 ][ 3 ] = CL;
}

static void GameTest0( gameConfig_t& config, std::vector< std::string >& commands ) {
	commands.reserve( 100 );
	commands.push_back( "p0a4" );
	commands.push_back( "p0a5" );
	commands.push_back( "p0a5" );
}