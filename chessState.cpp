#include "Chess.h"

#include <set>
#include <tuple>

// This class must *always* honor const-correctness upon destruction
class ScopedTempMove
{
public:

	ScopedTempMove( const ChessState* state, const Piece* piece, const num_t targetX, const num_t targetY ) : m_state( state )
	{
		Move( piece, targetX, targetY );
	}

	~ScopedTempMove()
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

void ChessState::SetHandle( const pieceHandle_t pieceHdl, const num_t x, const num_t y )
{
	if ( OnBoard( x, y ) == false ) {
		return;
	}
	grid[ y ][ x ] = pieceHdl;
}


pieceHandle_t ChessState::GetHandle( const num_t x, const num_t y ) const
{
	if ( OnBoard( x, y ) == false ) {
		return OffBoard;
	}
	return grid[ y ][ x ];
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
	if ( game->IsValidHandle( handle ) ) {
		return pieces[ handle ];
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
	if ( game->IsValidHandle( handle ) == false ) {
		return nullptr;
	}
	return pieces[ handle ];
}


moveType_t ChessState::IsLegalMove( const Piece* piece, const num_t targetX, const num_t targetY ) const
{
	moveType_t moveType = moveType_t::NONE;

	// 1. In bounds
	if ( OnBoard( targetX, targetY ) == false ) {
		return moveType_t::NONE;
	}
	if ( OnBoard( piece->x, piece->y ) == false ) {
		return moveType_t::NONE;
	}
	
	// 2. Quick test against possible moves using a hypothetical superset
#if USE_MOVE_CACHE_TEST
	const MoveCache& superset = piece->GetMoveCache();

	const num_t localX = ( targetX - piece->x );
	const num_t localY = ( targetY - piece->y ) * piece->GetTeamDirection();

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
		const pieceHandle_t kingHdl = game->FindPiece( piece->team, pieceType_t::KING, 0 );
		const Piece* king = GetPiece( kingHdl );

		// Temporarily mutates state, but honors function's const-contract
		ScopedTempMove tempMove( this, piece, targetX, targetY );

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
	targetPiece->RemoveFromPlay();

	const int32_t index			= static_cast<int32_t>( targetPiece->team );
	const int32_t attackerIndex	= static_cast<int32_t>( attacker );

	num_t& capturedCount		= teams[ attackerIndex ].capturedCount;
	num_t& playCount			= teams[ index ].livingCount;
	pieceHandle_t* teamPieces	= teams[ index ].pieces;

	teams[ attackerIndex ].captured[ capturedCount ] = targetPiece->handle;
	++capturedCount;

	for ( int32_t i = 0; i < playCount; ++i )
	{
		if ( teamPieces[ i ] == targetPiece->handle )
		{
			teamPieces[ i ] = teamPieces[ playCount - 1 ];
			teamPieces[ playCount - 1 ] = NoPiece;
			--playCount;
		}
	}
	return;
}


bool ChessState::IsOpenToAttack( const Piece* targetPiece ) const
{
	return IsOpenToAttackAt( targetPiece, targetPiece->x, targetPiece->y );
}


bool ChessState::IsOpenToAttackAt( const Piece* targetPiece, const num_t x, const num_t y ) const
{
	if ( OnBoard( x, y ) == false ) {
		return false;
	}
	const teamCode_t opposingTeam = ChessEngine::GetOpposingTeam( targetPiece->team );
	const int32_t index = static_cast<int32_t>( opposingTeam );

	for ( int32_t i = 0; i < teams[ index ].livingCount; ++i )
	{
		const Piece* piece = GetPiece( teams[ index ].pieces[ i ] );

#if USE_MOVE_CACHE_TEST
		const MoveCache& superset = piece->GetMoveCache();

		const num_t localX = ( x - piece->x );
		const num_t localY = ( y - piece->y ) * piece->GetTeamDirection();

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

	const pieceHandle_t handle = grid[ y ][ x ];
	if ( handle == NoPiece ) {
		return false;
	}
	return ( pieces[ handle ]->team == team );
}


bool ChessState::IsKingCaptured( const teamCode_t checkedTeamCode ) const
{
	const pieceHandle_t kingHdl = game->FindPiece( checkedTeamCode, pieceType_t::KING, 0 );
	const Piece* king = GetPiece( kingHdl );

	return ( king == nullptr ) ? true : false;
}


bool ChessState::IsChecked( const teamCode_t checkedTeamCode ) const
{
	const pieceHandle_t kingHdl = game->FindPiece( checkedTeamCode, pieceType_t::KING, 0 );
	const Piece* king = GetPiece( kingHdl );

	// Game is over, consider checked too
	if ( king == nullptr ) {
		return true;
	}

	return IsOpenToAttack( king );
}


bool ChessState::IsCheckMate( const Piece* attacker, const teamCode_t checkedTeamCode ) const
{
	const pieceHandle_t kingHdl = game->FindPiece( checkedTeamCode, pieceType_t::KING, 0 );
	const Piece* king = GetPiece( kingHdl );

	// King was captured after last move
	if ( king == nullptr ) {
		return true;
	}

	// King is pinned
	const int32_t actionCount = king->GetActionCount();
	for ( int32_t action = 0; action < actionCount; ++action )
	{
		num_t nextX = king->x;
		num_t nextY = king->y;
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
		if ( attacker->InActionPath( actionNum, king->x, king->y ) == false ) {
			continue;
		}

		num_t nextX = attacker->x;
		num_t nextY = attacker->y;
		const int32_t maxSteps = attacker->GetActions()[ actionNum ].maxSteps;

		// Represents kill action
		attackSquares.insert( move_t( nextX, nextY ) );

		for ( int32_t step = 1; step <= maxSteps; ++step )
		{
			attacker->CalculateStep( actionNum, nextX, nextY );

			if ( nextX == king->x && nextY == king->y ) {
				break;
			}

			attackSquares.insert( move_t( nextX, nextY ) );
		}
	}

	const team_t defenderTeam = teams[ static_cast<int32_t>( checkedTeamCode ) ];

	for ( int32_t defenderIx = 0; defenderIx < defenderTeam.livingCount; ++defenderIx )
	{
		const Piece* defenderPiece = GetPiece( defenderTeam.pieces[ defenderIx ] );
		const int32_t actionCount = defenderPiece->GetActionCount();

		if ( defenderPiece->handle == kingHdl ) {
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
	const team_t team = teams[ (int32_t)teamCode ];

	for ( int32_t pieceIx = 0; pieceIx < team.livingCount; ++pieceIx )
	{
		const Piece* piece = GetPiece( team.pieces[ pieceIx ] );
		const int32_t actionCount = piece->GetActionCount();

		for ( int32_t actionNum = 0; actionNum < actionCount; ++actionNum )
		{
			num_t nextX = piece->x;
			num_t nextY = piece->y;
			const int32_t maxSteps = piece->GetActions()[ actionNum ].maxSteps;

			for ( int32_t step = 1; step <= maxSteps; ++step )
			{
				piece->CalculateStep( actionNum, nextX, nextY );

				if ( IsLegalMove( piece, nextX, nextY ) != moveType_t::NONE )
				{
					const pieceHandle_t kingHdl = game->FindPiece( teamCode, pieceType_t::KING, 0 );
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
	Piece* piece = GetPiece( enpassantPawn );
	if ( piece != nullptr )
	{
		const Pawn* pawn = reinterpret_cast<const Pawn*>( piece );
		const num_t x = pawn->x;
		const num_t y = ( pawn->y - pawn->GetTeamDirection() );

		const bool wasEnpassant = ( x == targetX ) && ( y == targetY );

		if ( wasEnpassant ) {
			return piece;
		}
	}
	return nullptr;
}


void ChessState::CountTeamPieces( const bool initialCount )
{
	for ( int32_t i = 0; i < TeamCount; ++i )
	{
		for ( int32_t j = 0; j < static_cast<int32_t>( pieceType_t::COUNT ); ++j ) {
			teams[ i ].typeCounts[ j ] = 0;
		}

		for ( int32_t j = 0; j < teams[ i ].livingCount; ++j )
		{
			Piece* piece = GetPiece( teams[ i ].pieces[ j ] );
			const pieceType_t type = piece->type;
			const int32_t index = static_cast<int32_t>( type );

			teams[ i ].typeCounts[ index ]++;
		}

		for ( int32_t j = 0; j < teams[ i ].capturedCount; ++j )
		{
			const pieceType_t type = GetPiece( teams[ i ].captured[ j ] )->type;
			const int32_t index = static_cast<int32_t>( type );
			teams[ i ].captureTypeCounts[ index ]++;
		}
	}

	if( initialCount )
	{
		for ( int32_t i = 0; i < TeamCount; ++i )
		{
			num_t typeCounts[ (int32_t)pieceType_t::COUNT ] = {};

			for ( int32_t j = 0; j < teams[ i ].livingCount; ++j )
			{
				Piece* piece = GetPiece( teams[ i ].pieces[ j ] );
				const pieceType_t type = piece->type;
				const int32_t index = static_cast<int32_t>( type );

				piece->instance = typeCounts[ index ];

				typeCounts[ index ]++;
			}
		}
	}
}