#include "chess.h"
#include "game.h"
#include "chessState.h"
#include "piece.h"

#include <iostream>

bool Chess::PerformMoveAction( const pieceHandle_t pieceHdl, const int targetX, const int targetY ) {
	Piece* piece = s->GetPiece( pieceHdl );
	if ( piece == nullptr ) {
		return false;
	}
	const bool legalMove = s->IsLegalMove( piece, targetX, targetY );
	if ( legalMove == false ) {
		return false;
	}
	piece->Move( targetX, targetY );

	const teamCode_t opposingTeam = GetOpposingTeam( piece->team );
	if ( s->FindCheckMate( opposingTeam ) ) {
		winner = piece->team;
	}
	s->CountTeamPieces();
	return true;
}

pieceHandle_t Chess::FindPiece( const teamCode_t team, const pieceType_t type, const int instance ) {
	if ( ( team == teamCode_t::NONE ) || ( type == pieceType_t::NONE ) ) {
		return NoPiece;
	}
	for ( int i = 0; i < s->pieceNum; ++i ) {
		const bool teamsMatch = ( s->pieces[ i ]->team == team );
		const bool piecesMatch = ( s->pieces[ i ]->type == type );
		const bool instanceMatch = ( s->pieces[ i ]->instance == instance );
		if ( teamsMatch && piecesMatch && instanceMatch ) {
			return i;
		}
	}
	return NoPiece;
}

void Chess::SetBoard( const gameConfig_t& cfg ) {
	for ( int i = 0; i < BoardSize; ++i ) {
		for ( int j = 0; j < BoardSize; ++j ) {
			s->grid[ i ][ j ] = NoPiece;
			const pieceType_t pieceType = cfg.board[ i ][ j ].piece;
			const teamCode_t teamCode = cfg.board[ i ][ j ].team;
			Piece* piece = Chess::CreatePiece( pieceType, teamCode );
			if ( piece != nullptr ) {
				s->EnterPieceInGame( piece, j, i );
			}
		}
	}
}

bool ChessState::IsValidHandle( const pieceHandle_t handle ) const {
	if ( handle == NoPiece ) {
		return false;
	}
	if ( ( handle < 0 ) && ( handle >= pieceNum ) ) {
		return false;
	}
	return true;
}

Piece* Chess::CreatePiece( const pieceType_t pieceType, const teamCode_t teamCode ) {
	switch ( pieceType ) {
	case pieceType_t::PAWN:		return new Pawn( teamCode );
	case pieceType_t::ROOK:		return new Rook( teamCode );
	case pieceType_t::KNIGHT:	return new Knight( teamCode );
	case pieceType_t::BISHOP:	return new Bishop( teamCode );
	case pieceType_t::QUEEN:	return new Queen( teamCode );
	case pieceType_t::KING:		return new King( teamCode );
	}
	return nullptr;
}