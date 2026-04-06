#include "Chess.h"

#include <iostream>

bool ChessEngine::PerformMoveAction( const pieceHandle_t pieceHdl, const num_t targetX, const num_t targetY )
{
	Piece* piece = s.GetPiece( pieceHdl );
	if ( piece == nullptr ) {
		return false;
	}

	const moveType_t legalMove = s.IsLegalMove( piece, targetX, targetY );
	if ( legalMove == moveType_t::NONE ) {
		return false;
	}
	piece->Move( legalMove, targetX, targetY );

	return true;
}


void ChessEngine::CalculateGameState( const pieceHandle_t movedPieceHdl )
{
	Piece* piece = s.GetPiece( movedPieceHdl );
	if ( piece == nullptr ) {
		return;
	}

	// Check / Checkmate
	{
		const teamCode_t opposingTeam = GetOpposingTeam( piece->team );

		const pieceHandle_t kingHdl = FindPiece( opposingTeam, pieceType_t::KING, 0 );
		const Piece* king = s.GetPiece( kingHdl );

		checkedTeam = teamCode_t::NONE;
		if ( king == nullptr )
		{
			winner = piece->team;
		}

		const bool notCapturedAfterMove = ( king != nullptr );

		if ( notCapturedAfterMove && s.IsOpenToAttack( king ) )
		{
			checkedTeam = opposingTeam;

			if ( s.IsCheckMate( piece, opposingTeam ) )
			{
				winner = piece->team;
			}
		}
		else
		{
			if ( s.IsStalemate( opposingTeam ) )
			{
				stalemate = true;
			}
		}
	}
	s.CountTeamPieces( false );

	currentTurn = ( currentTurn == teamCode_t::WHITE ) ? teamCode_t::BLACK : teamCode_t::WHITE;

	++turnCount;
}


pieceHandle_t ChessEngine::FindPiece( const teamCode_t team, const pieceType_t type, const num_t instance )
{
	if ( ( team == teamCode_t::NONE ) || ( type == pieceType_t::NONE ) ) {
		return NoPiece;
	}

	for ( int32_t i = 0; i < GetPieceCount(); ++i )
	{
		const bool teamsMatch = ( s.pieces[ i ]->team == team );
		const bool piecesMatch = ( s.pieces[ i ]->type == type );
		const bool instanceMatch = ( s.pieces[ i ]->instance == instance );

		if ( teamsMatch && piecesMatch && instanceMatch ) {
			return i;
		}
	}
	return NoPiece;
}


void ChessEngine::SetBoard( const gameConfig_t& cfg )
{
	turnCount = 0;
	currentTurn = teamCode_t::WHITE;

	for ( int32_t i = 0; i < BoardSize; ++i )
	{
		for ( int32_t j = 0; j < BoardSize; ++j )
		{
			s.grid[ i ][ j ] = NoPiece;

			const pieceType_t pieceType = cfg.board[ i ][ j ].pieceType;
			const teamCode_t teamCode = cfg.board[ i ][ j ].team;

			Piece* piece = ChessEngine::CreatePiece( pieceType, teamCode );

			if ( piece != nullptr ) {
				EnterPieceInGame( piece, j, i );
			}
		}
	}
}


void ChessEngine::EnterPieceInGame( Piece* piece, const num_t x, const num_t y )
{
	s.pieces[ pieceNum ] = piece;
	s.pieces[ pieceNum ]->BindBoard( &s, pieceNum );
	s.pieces[ pieceNum ]->PlaceAt( x, y );

	const int32_t teamIndex = static_cast<int32_t>( piece->team );
	const int32_t pieceIndex = s.teams[ teamIndex ].livingCount;

	s.teams[ teamIndex ].pieces[ pieceIndex ] = pieceNum;
	++s.teams[ teamIndex ].livingCount;
	++pieceNum;
}


bool ChessEngine::IsValidHandle( const pieceHandle_t handle ) const
{
	if ( handle == NoPiece ) {
		return false;
	}

	if ( ( handle < 0 ) || ( handle >= GetPieceCount() ) ) {
		return false;
	}
	return true;
}


pieceInfo_t ChessEngine::GetInfo( const pieceHandle_t pieceType ) const
{
	pieceInfo_t info;
	const Piece* piece = s.GetPiece( pieceType );

	if ( piece != nullptr )
	{
		info.pieceType = piece->type;
		info.team = piece->team;
		info.instance = piece->instance;
		info.onBoard = true;
	}
	else
	{
		info.pieceType = pieceType_t::NONE;
		info.team = teamCode_t::NONE;
		info.instance = 0;
		info.onBoard = false;
	}
	return info;
}


pieceInfo_t ChessEngine::GetInfo( const num_t x, const num_t y ) const
{
	pieceInfo_t info;
	const Piece* piece = s.GetPiece( x, y );

	if ( piece != nullptr )
	{
		info.pieceType = piece->type;
		info.team = piece->team;
		info.instance = piece->instance;
		info.isPiece = true;
		info.onBoard = true;
	}
	else
	{
		info.pieceType = pieceType_t::NONE;
		info.team = teamCode_t::NONE;
		info.instance = 0;
		info.isPiece = false;
		info.onBoard = s.OnBoard( x, y );
	}
	return info;
}


bool ChessEngine::GetLocation( const pieceHandle_t pieceType, num_t& x, num_t& y ) const
{
	const Piece* piece = s.GetPiece( pieceType );

	if ( piece != nullptr )
	{
		x = piece->x;
		y = piece->y;
		return true;
	}
	else
	{
		x = -1;
		y = -1;
		return false;
	}
}


Piece* ChessEngine::CreatePiece( const pieceType_t pieceType, const teamCode_t teamCode )
{
	switch ( pieceType )
	{
		case pieceType_t::PAWN:		return new Pawn( teamCode );
		case pieceType_t::ROOK:		return new Rook( teamCode );
		case pieceType_t::KNIGHT:	return new Knight( teamCode );
		case pieceType_t::BISHOP:	return new Bishop( teamCode );
		case pieceType_t::QUEEN:	return new Queen( teamCode );
		case pieceType_t::KING:		return new King( teamCode );
	}
	return nullptr;
}


void ChessEngine::DestroyPiece( Piece*& piece )
{
	if( piece == nullptr ) {
		return;
	}

	piece->RemoveFromPlay();
	delete piece;
	piece = nullptr;
}