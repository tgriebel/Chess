#include "common.h"
#include "chess.h"
#include "chessState.h"
#include "piece.h"

#include <iostream>

bool Chess::PerformMoveAction( const pieceHandle_t pieceHdl, const int targetX, const int targetY ) {
	Piece* piece = s.GetPiece( pieceHdl );
	if ( piece == nullptr ) {
		return false;
	}
	const bool legalMove = s.IsLegalMove( piece, targetX, targetY );
	if ( legalMove == false ) {
		return false;
	}
	piece->Move( targetX, targetY );

	const teamCode_t opposingTeam = GetOpposingTeam( piece->team );
	if ( s.FindCheckMate( opposingTeam ) ) {
		winner = piece->team;
	}
	s.CountTeamPieces();
	return true;
}

pieceHandle_t Chess::FindPiece( const teamCode_t team, const pieceType_t type, const int instance ) {
	if ( ( team == teamCode_t::NONE ) || ( type == pieceType_t::NONE ) ) {
		return NoPiece;
	}
	for ( int i = 0; i < GetPieceCount(); ++i ) {
		const bool teamsMatch = ( s.pieces[ i ]->team == team );
		const bool piecesMatch = ( s.pieces[ i ]->type == type );
		const bool instanceMatch = ( s.pieces[ i ]->instance == instance );
		if ( teamsMatch && piecesMatch && instanceMatch ) {
			return i;
		}
	}
	return NoPiece;
}

void Chess::SetBoard( const gameConfig_t& cfg ) {
	for ( int i = 0; i < BoardSize; ++i ) {
		for ( int j = 0; j < BoardSize; ++j ) {
			s.grid[ i ][ j ] = NoPiece;
			const pieceType_t pieceType = cfg.board[ i ][ j ].piece;
			const teamCode_t teamCode = cfg.board[ i ][ j ].team;
			Piece* piece = Chess::CreatePiece( pieceType, teamCode );
			if ( piece != nullptr ) {
				EnterPieceInGame( piece, j, i );
			}
		}
	}
}

void Chess::EnterPieceInGame( Piece* piece, const int x, const int y ) {
	s.pieces[ pieceNum ] = piece;
	s.pieces[ pieceNum ]->BindBoard( &s, pieceNum );
	s.pieces[ pieceNum ]->Set( x, y );

	const int teamIndex = static_cast<int>( piece->team );
	const int pieceIndex = s.teams[ teamIndex ].livingCount;
	s.teams[ teamIndex ].pieces[ pieceIndex ] = pieceNum;
	++s.teams[ teamIndex ].livingCount;
	++pieceNum;
}

bool Chess::IsValidHandle( const pieceHandle_t handle ) const {
	if ( handle == NoPiece ) {
		return false;
	}
	if ( ( handle < 0 ) && ( handle >= GetPieceCount() ) ) {
		return false;
	}
	return true;
}

pieceInfo_t Chess::GetInfo( const int x, const int y ) const {
	pieceInfo_t info;
	const Piece* piece = s.GetPiece( x, y );
	if ( piece != nullptr ) {
		info.piece = piece->type;
		info.team = piece->team;
		info.instance = piece->instance;
		info.onBoard = true;
	} else {
		info.piece = pieceType_t::NONE;
		info.team = teamCode_t::NONE;
		info.instance = 0;
		info.onBoard = false;
	}
	return info;
}

bool Chess::GetLocation( const pieceHandle_t pieceType, int& x, int& y ) const {
	const Piece* piece = s.GetPiece( pieceType );
	if ( piece != nullptr ) {
		x = piece->x;
		y = piece->y;
		return true;
	} else {
		x = -1;
		y = -1;
		return false;
	}
}

pieceInfo_t Chess::GetInfo( const pieceHandle_t pieceType ) const {
	pieceInfo_t info;
	const Piece* piece = s.GetPiece( pieceType );
	if ( piece != nullptr ) {
		info.piece = piece->type;
		info.team = piece->team;
		info.instance = piece->instance;
		info.onBoard = true;
	} else {
		info.piece = pieceType_t::NONE;
		info.team = teamCode_t::NONE;
		info.instance = 0;
		info.onBoard = false;
	}
	return info;
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