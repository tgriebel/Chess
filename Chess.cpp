#include "Chess.h"

#include <iostream>

bool ChessEngine::PerformMoveAction( const pieceHandle_t pieceHdl, const num_t targetX, const num_t targetY )
{
	Piece* piece = m_state.GetPiece( pieceHdl );
	if ( piece == nullptr ) {
		return false;
	}

	const moveType_t legalMove = m_state.IsLegalMove( piece, targetX, targetY );
	if ( legalMove == moveType_t::NONE ) {
		return false;
	}
	piece->Move( legalMove, targetX, targetY );

	CalculateGameState( legalMove, pieceHdl );

	return true;
}


void ChessEngine::CalculateGameState( const moveType_t& moveType, const pieceHandle_t movedPieceHdl )
{
	Piece* piece = m_state.GetPiece( movedPieceHdl );
	if ( piece == nullptr ) {
		return;
	}

	// Check / Checkmate
	{
		const teamCode_t opposingTeam = GetOpposingTeam( piece->team );

		const pieceHandle_t kingHdl = FindPiece( opposingTeam, pieceType_t::KING, 0 );
		const Piece* king = m_state.GetPiece( kingHdl );

		m_checkedTeam = teamCode_t::NONE;
		if ( king == nullptr )
		{
			m_winner = piece->team;
		}

		const bool notCapturedAfterMove = ( king != nullptr );

		if ( notCapturedAfterMove && m_state.IsOpenToAttack( king ) )
		{
			m_checkedTeam = opposingTeam;

			if ( m_state.IsCheckMate( piece, moveType, opposingTeam ) )
			{
				m_winner = piece->team;
			}
		}
		else
		{
			if ( m_state.IsStalemate( opposingTeam ) )
			{
				m_stalemate = true;
			}
		}
	}

	m_currentTurn = ( m_currentTurn == teamCode_t::WHITE ) ? teamCode_t::BLACK : teamCode_t::WHITE;

	++m_turnCount;
}


int32_t ChessEngine::Search( int32_t depth )
{
	// Perft search

	int32_t nodes = 0;

	if ( depth == 0 ) {
		return 1;
	}

	position_t moveList[ 256 ] = {};

	const int32_t pieceCount = GetPieceCount();

	ChessState searchState;

	for( int32_t pieceId = 0; pieceId < pieceCount; ++pieceId )
	{
		Piece* piece = m_pieces[ pieceId ];

		const int32_t moveCount = piece->ComputeAllMoveActions( moveList );
		if( moveCount == 0 ) {
			continue;
		}

		for ( int32_t moveIx = 0; moveIx < moveCount; ++moveIx )
		{
			// FIXME: Legality check. Does redundant work so optimize this later
			moveType_t move = m_state.IsLegalMove( piece, moveList[ moveIx ].x, moveList[ moveIx ].y );
			if ( move == moveType_t::NONE )
			{
				continue;
			}

			// Make move
			{
				searchState.CopyFrom( m_state );
				piece->BindBoard( &searchState, piece->m_handle );
				piece->PlaceAt( piece->X(), piece->Y() );

				piece->Move( move, moveList[ moveIx ].x, moveList[ moveIx ].y );
			}

			nodes += Search( depth - 1 );
			
			// Undo move
			{
				// Nothing, using a copied state currently
			}
		}
	}
	return nodes;
}


pieceHandle_t ChessEngine::FindPiece( const teamCode_t team, const pieceType_t type, const num_t instance )
{
	if ( ( team == teamCode_t::NONE ) || ( type == pieceType_t::NONE ) ) {
		return NoPiece;
	}

	for ( int32_t i = 0; i < GetPieceCount(); ++i )
	{
		const bool teamsMatch = ( m_pieces[ i ]->team == team );
		const bool piecesMatch = ( m_pieces[ i ]->type == type );
		const bool instanceMatch = ( m_pieces[ i ]->GetInstanceNumber() == instance );

		if ( teamsMatch && piecesMatch && instanceMatch ) {
			return i;
		}
	}
	return NoPiece;
}


void ChessEngine::SetBoard( const gameConfig_t& cfg )
{
	m_turnCount = 0;
	m_currentTurn = teamCode_t::WHITE;

	for ( int32_t i = 0; i < BoardSize; ++i )
	{
		for ( int32_t j = 0; j < BoardSize; ++j )
		{
			m_state.m_grid[ i ][ j ] = NoPiece;

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
	m_pieces[ m_pieceNum ] = piece;
	m_pieces[ m_pieceNum ]->BindBoard( &m_state, m_pieceNum );
	m_pieces[ m_pieceNum ]->PlaceAt( x, y );

	const num_t teamIndex = static_cast<num_t>( piece->team );

	team_t& team = m_state.m_teams[ teamIndex ];

	const num_t pieceIndex = team.livingCount;
	const num_t pieceTypeIndex = static_cast<num_t>( piece->type );

	team.pieces[ pieceIndex ] = m_pieceNum;
	++team.livingCount;
	++m_pieceNum;

	piece->SetInstanceNumber( team.typeCounts[ pieceTypeIndex ] );

	++team.typeCounts[ pieceTypeIndex ];
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
	const Piece* piece = m_state.GetPiece( pieceType );

	if ( piece != nullptr )
	{
		info.pieceType = piece->type;
		info.team = piece->team;
		info.instance = piece->GetInstanceNumber();
		info.isPiece = true;
		info.onBoard = true;
	}
	else
	{
		info.pieceType = pieceType_t::NONE;
		info.team = teamCode_t::NONE;
		info.instance = 0;
		info.isPiece = false;
		info.onBoard = false;
	}
	return info;
}


pieceInfo_t ChessEngine::GetInfo( const num_t x, const num_t y ) const
{
	pieceInfo_t info;
	const Piece* piece = m_state.GetPiece( x, y );

	if ( piece != nullptr )
	{
		info.pieceType = piece->type;
		info.team = piece->team;
		info.instance = piece->GetInstanceNumber();
		info.isPiece = true;
		info.onBoard = true;
	}
	else
	{
		info.pieceType = pieceType_t::NONE;
		info.team = teamCode_t::NONE;
		info.instance = 0;
		info.isPiece = false;
		info.onBoard = m_state.OnBoard( x, y );
	}
	return info;
}


bool ChessEngine::GetLocation( const pieceHandle_t pieceType, num_t& x, num_t& y ) const
{
	const Piece* piece = m_state.GetPiece( pieceType );

	if ( piece != nullptr )
	{
		x = piece->X();
		y = piece->Y();
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
	Piece* piece = nullptr;

	switch ( pieceType )
	{
		case pieceType_t::PAWN:
		{
			piece = new Piece();

			piece->type = pieceType_t::PAWN;
			piece->team = teamCode;
			piece->m_numActions = static_cast<int32_t>( moveType_t::PAWN_ACTIONS );
			piece->m_actions = PawnActions;
			piece->m_moveSuperset = &PawnMoveSuperset;

			piece->m_teamDirection = ( teamCode == teamCode_t::WHITE ) ? -1 : 1;

			piece->FillMoveCache();
		} break;
		
		case pieceType_t::ROOK:
		{
			piece = new Piece();

			piece->type = pieceType_t::ROOK;
			piece->team = teamCode;
			piece->m_numActions = static_cast<int32_t>( moveType_t::ROOK_ACTIONS );
			piece->m_actions = RookActions;
			piece->m_moveSuperset = &RookMoveSuperset;

			piece->FillMoveCache();
		} break;

		case pieceType_t::KNIGHT:
		{
			piece = new Piece();

			piece->type = pieceType_t::KNIGHT;
			piece->team = teamCode;
			piece->m_numActions = static_cast<int32_t>( moveType_t::KNIGHT_ACTIONS );
			piece->m_actions = KnightActions;
			piece->m_moveSuperset = &KnightMoveSuperset;

			piece->FillMoveCache();
		} break;

		case pieceType_t::BISHOP:
		{
			piece = new Piece();

			piece->type = pieceType_t::BISHOP;
			piece->team = teamCode;
			piece->m_numActions = static_cast<int32_t>( moveType_t::BISHOP_ACTIONS );
			piece->m_actions = BishopActions;
			piece->m_moveSuperset = &BishopMoveSuperset;

			piece->FillMoveCache();
		} break;

		case pieceType_t::QUEEN:
		{
			piece = new Piece();

			piece->type = pieceType_t::QUEEN;
			piece->team = teamCode;
			piece->m_numActions = static_cast<int32_t>( moveType_t::QUEEN_ACTIONS );
			piece->m_actions = QueenActions;
			piece->m_moveSuperset = &QueenMoveSuperset;

			piece->FillMoveCache();
		} break;
		
		case pieceType_t::KING:
		{
			piece = new Piece();

			piece->type = pieceType_t::KING;
			piece->team = teamCode;
			piece->m_numActions = static_cast<int32_t>( moveType_t::KING_ACTIONS );
			piece->m_actions = KingActions;
			piece->m_moveSuperset = &KingMoveSuperset;

			piece->FillMoveCache();
		} break;
	}
	return piece;
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