#pragma once
#include <string>
#include "common.h"
#include "piece.h"
#include "chess.h"

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

static resultCode_t TranslateActionCommand( const Chess& board, const teamCode_t team, const std::string& commandString, command_t& outCmd ) {
	if ( commandString.size() != 4 ) {
		return RESULT_INPUT_INVALID_COMMAND;
	}
	outCmd.team = team;
	outCmd.pieceType = GetPieceType( commandString[ 0 ] );
	outCmd.instance = commandString[ 1 ] - '0';
	if ( board.FindPiece( outCmd.team, outCmd.pieceType, outCmd.instance ) == NoPiece ) {
		return RESULT_INPUT_INVALID_PIECE;
	}
	outCmd.x = GetFileNum( commandString[ 2 ] );
	if ( ( outCmd.x < 0 ) && ( outCmd.x >= BoardSize ) ) {
		return RESULT_INPUT_INVALID_FILE;
	}
	outCmd.y = GetRankNum( commandString[ 3 ] );
	if ( ( outCmd.y < 0 ) && ( outCmd.y >= BoardSize ) ) {
		return RESULT_INPUT_INVALID_RANK;
	}
	return RESULT_SUCCESS;
}