#include "Chess.h"

#include <set>
#include <tuple>

// This class must *always* honor const-correctness upon destruction
class ScopedTempPlacement
{
public:

	ScopedTempPlacement( const ChessState* state, const Piece* piece, const num_t targetX, const num_t targetY ) : m_state( state )
	{
		Move( piece, targetX, targetY );
	}

	~ScopedTempPlacement()
	{
		const_cast<Piece*>( m_piece )->ReturnPlacement();
		if ( m_occupiedPiece != nullptr ) {
			const_cast<Piece*>( m_occupiedPiece )->ReturnPlacement();
		}
	}

private:

	void Move( const Piece* piece, const num_t targetX, const num_t targetY )
	{
		m_piece = piece;

		m_occupiedPiece = m_state->GetPiece( targetX, targetY );
		if ( m_occupiedPiece != nullptr ) {
			const_cast<Piece*>( m_occupiedPiece )->TempPlacement( -1, -1 );
		}

		const_cast<Piece*>( piece )->TempPlacement( targetX, targetY );
	}

	const ChessState* m_state;
	const Piece* m_piece = nullptr;
	const Piece* m_occupiedPiece = nullptr;
};


class ScopedSearch
{
public:

	ScopedSearch( const ChessState* state, const Piece* piece, const num_t targetX, const num_t targetY ) : m_state( state )
	{
		m_prevX = piece->X();
		m_prevY = piece->Y();

		m_piece = piece;

		m_occupiedPiece = m_state->GetPiece( targetX, targetY );
	}

	~ScopedSearch()
	{
		
	}

private:

	num_t m_prevX = 0;
	num_t m_prevY = 0;

	enum actionType_t
	{
		STANDARD,
		CAPTURE,
		PAWN,
		CASTLE,
	};

	struct captureAction_t
	{
		const Piece* piece;
		num_t prevX;
		num_t prevY;
		num_t pieceIndex;
	};


	struct pawnAction_t
	{
		const Piece* empassant;
	};


	struct castleAction_t
	{
		const Piece* rook;
		num_t rookPrevX;
		num_t rookPrevY;
	};

	union action_t
	{
		captureAction_t captureAction;
		pawnAction_t pawnAction;
		castleAction_t castleAction;
	};

	action_t action;
	actionType_t actionType;

	num_t moveCounter;

	const ChessState* m_state;
	const Piece* m_piece = nullptr;
	const Piece* m_occupiedPiece = nullptr;
};


void ChessState::SetHandle( const pieceHandle_t pieceHdl, const num_t x, const num_t y )
{
	if ( OnBoard( x, y ) == false ) {
		return;
	}
	m_grid[ y ][ x ] = pieceHdl;
}


pieceHandle_t ChessState::GetHandle( const num_t x, const num_t y ) const
{
	if ( OnBoard( x, y ) == false ) {
		return OffBoard;
	}
	return m_grid[ y ][ x ];
}


bool ChessState::OnBoard( const num_t x, const num_t y ) const
{
	return ( x >= 0 ) && ( x < BoardSize ) && ( y >= 0 ) && ( y < BoardSize );
}


const Piece* ChessState::GetPiece( const pieceHandle_t handle ) const
{
	return const_cast<ChessState*>( this )->GetPiece( handle );
}


Piece* ChessState::GetPiece( const pieceHandle_t handle )
{
	if ( m_game->IsValidHandle( handle ) ) {
		return m_pieces[ handle ];
	}
	return nullptr;
}


const Piece* ChessState::GetPiece( const num_t x, const num_t y ) const
{
	return const_cast<ChessState*>( this )->GetPiece( x, y );
}


Piece* ChessState::GetPiece( const num_t x, const num_t y )
{
	const pieceHandle_t handle = GetHandle( x, y );
	if ( m_game->IsValidHandle( handle ) == false ) {
		return nullptr;
	}
	return m_pieces[ handle ];
}


moveType_t ChessState::IsLegalMove( const Piece* piece, const num_t targetX, const num_t targetY ) const
{
	moveType_t moveType = moveType_t::NONE;

	// 1. In bounds
	if ( OnBoard( targetX, targetY ) == false ) {
		return moveType_t::NONE;
	}
	if ( OnBoard( piece->X(), piece->Y() ) == false ) {
		return moveType_t::NONE;
	}
	
	// 2. Quick test against possible moves using a hypothetical superset
#if USE_MOVE_CACHE_TEST
	const MoveCache& superset = piece->GetMoveCache();

	const num_t localX = ( targetX - piece->X() );
	const num_t localY = ( targetY - piece->Y() ) * piece->GetTeamDirection();

	if ( !superset.Test( localX, localY) ) {
		return moveType_t::NONE;
	}
#endif

	// 3. Check if the piece's actions can reach this location
	{
		const int32_t actionCount = piece->GetActionCount();

		for ( int32_t action = 0; action < actionCount; ++action )
		{
			if ( piece->InActionPath( action, targetX, targetY ) )
			{
				moveType = piece->GetActions()[ action ].type;
				break;
			}
		}

		if ( moveType == moveType_t::NONE ) {
			return moveType_t::NONE;
		}
	}

	// 4. It's illegal for any move to leave that team's king checked
	{
		const pieceHandle_t kingHdl = m_game->FindPiece( piece->team, pieceType_t::KING, 0 );
		const Piece* king = GetPiece( kingHdl );

		// Temporarily mutates state, but honors function's const-contract
		ScopedTempPlacement tempMove( this, piece, targetX, targetY );

		if ( IsOpenToAttack( king ) ) {
			return moveType_t::NONE;
		}
	}

	return moveType;
}


void ChessState::CapturePiece( const teamCode_t attacker, Piece* targetPiece )
{
	if ( targetPiece == nullptr ) {
		return;
	}

	const int32_t index				= static_cast<int32_t>( targetPiece->team );
	const int32_t pieceTypeIndex	= (int32_t)targetPiece->type;
	const int32_t attackerIndex		= static_cast<int32_t>( attacker );

	// Update attacker team stats
	{
		num_t* captured = m_teams[ attackerIndex ].captured;
		num_t* capturedTypeCount = m_teams[ attackerIndex ].captureTypeCounts;
		num_t& capturedCount = m_teams[ attackerIndex ].capturedCount;

		++capturedTypeCount[ pieceTypeIndex ];

		captured[ capturedCount ] = targetPiece->m_handle;
		++capturedCount;
	}

	// Update current team stats
	{
		num_t* teamPieces = m_teams[ index ].pieces;
		num_t* typeCounts = m_teams[ index ].typeCounts;
		num_t& livingCount = m_teams[ index ].livingCount;

		for ( int32_t i = 0; i < livingCount; ++i )
		{
			if ( teamPieces[ i ] == targetPiece->m_handle )
			{
				teamPieces[ i ] = teamPieces[ livingCount - 1 ];
				teamPieces[ livingCount - 1 ] = NoPiece;
				--livingCount;
				break;
			}
		}
		--typeCounts[ pieceTypeIndex ];
	}

	targetPiece->RemoveFromPlay();

	return;
}


void ChessState::ReverseCapturePiece( const teamCode_t attacker, Piece* targetPiece )
{
	if ( targetPiece == nullptr ) {
		return;
	}

	const int32_t index = static_cast<int32_t>( targetPiece->team );
	const int32_t pieceTypeIndex = (int32_t)targetPiece->type;
	const int32_t attackerIndex = static_cast<int32_t>( attacker );

	// Update attacker team stats
	{
		num_t* captured = m_teams[ attackerIndex ].captured;
		num_t* capturedTypeCount = m_teams[ attackerIndex ].captureTypeCounts;
		num_t& capturedCount = m_teams[ attackerIndex ].capturedCount;

		--capturedCount;
		captured[ capturedCount ] = NoPiece;

		--capturedTypeCount[ pieceTypeIndex ];	
	}

	// Update current team stats
	{
		num_t* teamPieces = m_teams[ index ].pieces;
		num_t* typeCounts = m_teams[ index ].typeCounts;
		num_t& livingCount = m_teams[ index ].livingCount;

		++livingCount;

		//for ( int32_t i = 0; i < livingCount; ++i )
		//{


		//	if ( teamPieces[ i ] == targetPiece->m_handle )
		//	{
		//		teamPieces[ i ] = teamPieces[ livingCount - 1 ];
		//		teamPieces[ livingCount - 1 ] = NoPiece;
		//		--livingCount;
		//		break;
		//	}
		//}
	}

	targetPiece->RemoveFromPlay();

	return;
}



bool ChessState::IsOpenToAttack( const Piece* targetPiece ) const
{
	return IsOpenToAttackAt( targetPiece, targetPiece->X(), targetPiece->Y() );
}


bool ChessState::IsOpenToAttackAt( const Piece* targetPiece, const num_t x, const num_t y ) const
{
	if ( OnBoard( x, y ) == false ) {
		return false;
	}
	const teamCode_t opposingTeam = ChessEngine::GetOpposingTeam( targetPiece->team );
	const int32_t index = static_cast<int32_t>( opposingTeam );

	for ( int32_t i = 0; i < m_teams[ index ].livingCount; ++i )
	{
		const Piece* piece = GetPiece( m_teams[ index ].pieces[ i ] );

#if USE_MOVE_CACHE_TEST
		const MoveCache& superset = piece->GetMoveCache();

		const num_t localX = ( x - piece->X() );
		const num_t localY = ( y - piece->Y() ) * piece->GetTeamDirection();

		if ( !superset.Test( localX, localY ) ) {
			continue;
		}
#endif

		const int32_t actionCount = piece->GetActionCount();

		for ( int32_t action = 0; action < actionCount; ++action )
		{
			if ( piece->InActionPath( action, x, y ) ) {
				return true;
			}
		}
	}
	return false;
}


bool ChessState::IsBlocked( const teamCode_t team, const num_t x, const num_t y ) const
{
	assert( OnBoard( x, y ) );

	const pieceHandle_t handle = m_grid[ y ][ x ];
	if ( handle == NoPiece ) {
		return false;
	}
	return ( m_pieces[ handle ]->team == team );
}


bool ChessState::IsKingCaptured( const teamCode_t checkedTeamCode ) const
{
	const pieceHandle_t kingHdl = m_game->FindPiece( checkedTeamCode, pieceType_t::KING, 0 );
	const Piece* king = GetPiece( kingHdl );

	return ( king == nullptr ) ? true : false;
}


bool ChessState::IsChecked( const teamCode_t checkedTeamCode ) const
{
	const pieceHandle_t kingHdl = m_game->FindPiece( checkedTeamCode, pieceType_t::KING, 0 );
	const Piece* king = GetPiece( kingHdl );

	// Game is over, consider checked too
	if ( king == nullptr ) {
		return true;
	}

	return IsOpenToAttack( king );
}


bool ChessState::IsCheckMate( const Piece* attacker, const teamCode_t checkedTeamCode ) const
{
	const pieceHandle_t kingHdl = m_game->FindPiece( checkedTeamCode, pieceType_t::KING, 0 );
	const Piece* king = GetPiece( kingHdl );

	// King was captured after last move
	if ( king == nullptr ) {
		return true;
	}

	// King is pinned
	const int32_t actionCount = king->GetActionCount();
	for ( int32_t action = 0; action < actionCount; ++action )
	{
		num_t nextX = king->m_x;
		num_t nextY = king->m_y;
		king->CalculateStep( action, nextX, nextY );

		if( IsLegalMove( king, nextX, nextY ) != moveType_t::NONE ) {
			return false;
		}
	}

	// Path of all attackers can be blocked
	typedef std::tuple<int32_t, int32_t> move_t;
	std::set<move_t> attackSquares;

	const int32_t attackerActionCount = attacker->GetActionCount();

	for ( int32_t actionNum = 0; actionNum < attackerActionCount; ++actionNum )
	{
		if ( attacker->InActionPath( actionNum, king->X(), king->Y() ) == false ) {
			continue;
		}

		num_t nextX = attacker->X();
		num_t nextY = attacker->Y();
		const int32_t maxSteps = attacker->GetActions()[ actionNum ].maxSteps;

		// Represents kill action
		attackSquares.insert( move_t( nextX, nextY ) );

		for ( int32_t step = 1; step <= maxSteps; ++step )
		{
			attacker->CalculateStep( actionNum, nextX, nextY );

			if ( nextX == king->X() && nextY == king->Y() ) {
				break;
			}

			attackSquares.insert( move_t( nextX, nextY ) );
		}
	}

	const team_t defenderTeam = m_teams[ static_cast<int32_t>( checkedTeamCode ) ];

	for ( int32_t defenderIx = 0; defenderIx < defenderTeam.livingCount; ++defenderIx )
	{
		const Piece* defenderPiece = GetPiece( defenderTeam.pieces[ defenderIx ] );
		const int32_t actionCount = defenderPiece->GetActionCount();

		if ( defenderPiece->m_handle == kingHdl ) {
			continue;
		}

		for ( int32_t actionNum = 0; actionNum < actionCount; ++actionNum )
		{
			for ( const move_t square : attackSquares )
			{
				if ( IsLegalMove( defenderPiece, std::get<0>( square ), std::get<1>( square ) ) != moveType_t::NONE )
				{
					return false;
				}
			}
		}
	}
	return true;
}


bool ChessState::IsStalemate( const teamCode_t teamCode ) const
{
	const team_t team = m_teams[ (int32_t)teamCode ];

	for ( int32_t pieceIx = 0; pieceIx < team.livingCount; ++pieceIx )
	{
		const Piece* piece = GetPiece( team.pieces[ pieceIx ] );
		const int32_t actionCount = piece->GetActionCount();

		for ( int32_t actionNum = 0; actionNum < actionCount; ++actionNum )
		{
			num_t nextX = piece->X();
			num_t nextY = piece->Y();
			const int32_t maxSteps = piece->GetActions()[ actionNum ].maxSteps;

			for ( int32_t step = 1; step <= maxSteps; ++step )
			{
				piece->CalculateStep( actionNum, nextX, nextY );

				if ( IsLegalMove( piece, nextX, nextY ) != moveType_t::NONE )
				{
					const pieceHandle_t kingHdl = m_game->FindPiece( teamCode, pieceType_t::KING, 0 );
					const Piece* king = GetPiece( kingHdl );

					if( IsOpenToAttackAt( king, nextX, nextY ) ) {
						continue;
					}

					// Need to do one final check that there aren't two kings
					//if ( GetInfo( nextX, nextY ).piece != pieceType_t::NONE )
					//{
					//	if( ( team.livingCount == 1 ) && teams[ (int32_t)GetInfo( nextX, nextY ).team ].livingCount == 2 ) {
					//		return true;
					//	}
					//}

					return false;
				}
			}
		}
	}

	return true;
}


Piece* ChessState::GetEnpassant( const num_t targetX, const num_t targetY )
{
	Piece* piece = GetPiece( m_enpassantPawn );
	if ( piece != nullptr )
	{
		const num_t x = piece->X();
		const num_t y = ( piece->Y() - piece->GetTeamDirection() );

		const bool wasEnpassant = ( x == targetX ) && ( y == targetY );

		if ( wasEnpassant ) {
			return piece;
		}
	}
	return nullptr;
}