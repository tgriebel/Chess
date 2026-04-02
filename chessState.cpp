#include "Chess.h"


void ChessState::SetHandle( const pieceHandle_t pieceHdl, const int32_t x, const int32_t y )
{
	if ( OnBoard( x, y ) == false ) {
		return;
	}
	grid[ y ][ x ] = pieceHdl;
}


pieceHandle_t ChessState::GetHandle( const int32_t x, const int32_t y ) const
{
	if ( OnBoard( x, y ) == false ) {
		return OffBoard;
	}
	return grid[ y ][ x ];
}


bool ChessState::OnBoard( const int32_t x, const int32_t y ) const
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


const Piece* ChessState::GetPiece( const int32_t x, const int32_t y ) const
{
	return const_cast<ChessState*>( this )->GetPiece( x, y );
}


Piece* ChessState::GetPiece( const int32_t x, const int32_t y )
{
	const pieceHandle_t handle = GetHandle( x, y );
	if ( game->IsValidHandle( handle ) == false ) {
		return nullptr;
	}
	return pieces[ handle ];
}


pieceInfo_t ChessState::GetInfo( const int32_t x, const int32_t y ) const
{
	return game->GetInfo( x, y );
}


bool ChessState::IsLegalMove( const Piece* piece, const int32_t targetX, const int32_t targetY ) const
{
	if ( OnBoard( targetX, targetY ) == false ) {
		return false;
	}
	if ( OnBoard( piece->x, piece->y ) == false ) {
		return false;
	}
	// Check piece actions
	bool isLegal = false;
	const int32_t actionCount = piece->GetActionCount();

	for ( int32_t action = 0; action < actionCount; ++action )
	{
		if ( piece->InActionPath( action, targetX, targetY ) ) {
			isLegal = true;
			break;
		}
	}

	if ( isLegal == false ) {
		return false;
	}

	// It's illegal for any move to leave that team's king checked
	const_cast<Piece*>( piece )->TempPlacement( -1, -1 );

	const pieceHandle_t kingHdl = game->FindPiece( piece->team, pieceType_t::KING, 0 );
	const Piece* king = GetPiece( kingHdl );

	if ( IsOpenToAttack( king ) ) {
		isLegal = false;
	}

	// Reset position to honor this function's const-contract
	const_cast<Piece*>( piece )->ReturnPlacement();

	return isLegal;
}


void ChessState::CapturePiece( const teamCode_t attacker, Piece* targetPiece )
{
	if ( targetPiece == nullptr ) {
		return;
	}
	targetPiece->RemoveFromPlay();

	const int32_t index			= static_cast<int32_t>( targetPiece->team );
	const int32_t attackerIndex	= static_cast<int32_t>( attacker );

	int32_t& capturedCount		= teams[ attackerIndex ].capturedCount;
	int32_t& playCount			= teams[ index ].livingCount;
	pieceHandle_t* pieces		= teams[ index ].pieces;

	teams[ attackerIndex ].captured[ capturedCount ] = targetPiece->handle;
	++capturedCount;

	for ( int32_t i = 0; i < playCount; ++i )
	{
		if ( pieces[ i ] == targetPiece->handle )
		{
			pieces[ i ] = pieces[ playCount - 1 ];
			--playCount;
		}
	}
	return;
}


void ChessState::PromotePawn( const pieceHandle_t pieceHdl )
{
	const Piece* piece = GetPiece( pieceHdl );
	if ( ( piece == nullptr ) || ( piece->type != pieceType_t::PAWN ) ) {
		return;
	}

	callbackEvent_t event;
	event.type = PAWN_PROMOTION;
	event.promotionType = pieceType_t::NONE;

	// A.I. can use a callback to run a heuristic (e.g. always pick Queen)
	// While a user needs to make their pick of piece
	if ( callback != nullptr ) {
		( *callback )( event );
	}

	bool invalidChoice = true;
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::QUEEN );
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::KNIGHT );
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::BISHOP );
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::ROOK );

	if ( invalidChoice ) {
		event.promotionType = pieceType_t::QUEEN;
	}

	const teamCode_t team = piece->team;
	const int32_t x = piece->x;
	const int32_t y = piece->y;

	pieces[ pieceHdl ]->RemoveFromPlay();

	pieces[ pieceHdl ] = ChessEngine::CreatePiece( event.promotionType, team );
	pieces[ pieceHdl ]->BindBoard( this, pieceHdl );
	pieces[ pieceHdl ]->Move( x, y );
}


bool ChessState::IsOpenToAttack( const Piece* targetPiece ) const
{
	return IsOpenToAttackAt( targetPiece, targetPiece->x, targetPiece->y );
}


bool ChessState::IsOpenToAttackAt( const Piece* targetPiece, const int32_t x, const int32_t y ) const
{
	if ( OnBoard( x, y ) == false ) {
		return false;
	}
	const teamCode_t opposingTeam = ChessEngine::GetOpposingTeam( targetPiece->team );
	const int32_t index = static_cast<int32_t>( opposingTeam );

	for ( int32_t i = 0; i < teams[ index ].livingCount; ++i )
	{
		const Piece* piece = GetPiece( teams[ index ].pieces[ i ] );
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
		int32_t nextX = king->x;
		int32_t nextY = king->y;
		king->CalculateStep( action, nextX, nextY );

		if ( OnBoard( nextX, nextY ) == false ) {
			continue;
		}

		const Piece* piece = GetPiece( nextX, nextY );

		const bool pieceOccupiesTarget = ( piece != nullptr );
		const bool pieceIsFriendly = pieceOccupiesTarget && ( piece->team == king->team );

		if ( pieceIsFriendly ) {
			continue;
		}

		if( pieceOccupiesTarget ) {
			const_cast<Piece*>( piece )->TempPlacement( -1, - 1 );
		}
		const_cast<Piece*>( king )->TempPlacement( nextX, nextY );

		const bool isOpenToAttack = IsOpenToAttackAt( king, nextX, nextY );

		if ( pieceOccupiesTarget ) {
			const_cast<Piece*>( piece )->ReturnPlacement();
		}
		const_cast<Piece*>( king )->ReturnPlacement();
			
		if ( isOpenToAttack == false ) {
			return false;
		}
	}

	// Path of all attackers can be blocked
	const teamCode_t opposingTeamCode = ChessEngine::GetOpposingTeam( checkedTeamCode );

	const team_t defenderTeam = teams[ static_cast<int32_t>( checkedTeamCode ) ];
	const team_t attackerTeam = teams[ static_cast<int32_t>( opposingTeamCode ) ];

	struct attackerPath_t
	{
		pieceHandle_t	piece;
		int32_t			action;
	};

	int32_t totalAttackers = 0;
	int32_t blockedAttackers = 0;
	attackerPath_t attackerPaths[ TeamPieceCount ];

	for ( int32_t attackerIx = 0; attackerIx < attackerTeam.livingCount; ++attackerIx )
	{
		const Piece* attackerPiece = GetPiece( attackerTeam.pieces[ attackerIx ] );
		const int32_t attackerActionCount = attackerPiece->GetActionCount();

		for ( int32_t attackerAction = 0; attackerAction < attackerActionCount; ++attackerAction )
		{
			if ( attackerPiece->InActionPath( attackerAction, king->x, king->y ) )
			{
				attackerPaths[ totalAttackers ].piece = attackerIx;
				attackerPaths[ totalAttackers ].action = attackerAction;
				++totalAttackers;
				continue;
			}
		}
	}

	// FIXME: Rewrite this. Temp write all possible defender moves to grid then check attacker paths
	for ( int32_t attackerIx = 0; attackerIx < totalAttackers; ++attackerIx )
	{
		const attackerPath_t& attackerPath = attackerPaths[ attackerIx ];

		const Piece* attackerPiece = GetPiece( attackerPath.piece );

		moveAction_t path[ BoardSize ];
		const int32_t pathSteps = attackerPiece->GetActionPath( attackerPath.action, path );

		for ( int32_t step = 0; step < pathSteps; ++step )
		{
			bool blocked = false;
			for ( int32_t defenderIx = 0; defenderIx < defenderTeam.livingCount; ++defenderIx )
			{			
				const Piece* defenderPiece = GetPiece( defenderTeam.pieces[ defenderIx ] );
				const int32_t defenderActionCount = defenderPiece->GetActionCount();

				for ( int32_t defenderAction = 0; defenderAction < defenderActionCount; ++defenderAction )
				{
					// Can block
					if ( defenderPiece->InActionPath( defenderAction, path[ step ].x, path[ step ].y ) )
					{
						++blockedAttackers;
						blocked = true;
						break;
					}
				}
				if( blocked ) {
					break;
				}
			}
			if ( blocked ) {
				break;
			}
		}
	}
	return true;
}


pieceHandle_t ChessState::GetEnpassant( const int32_t targetX, const int32_t targetY ) const
{
	const Piece* piece = GetPiece( enpassantPawn );
	if ( piece != nullptr )
	{
		const Pawn* pawn = reinterpret_cast<const Pawn*>( piece );
		const int32_t x = pawn->x;
		const int32_t y = ( pawn->y - pawn->GetTeamDirection() );

		const bool wasEnpassant = ( x == targetX ) && ( y == targetY );

		if ( wasEnpassant ) {
			return piece->handle;
		}
	}
	return NoPiece;
}


void ChessState::CountTeamPieces()
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

			piece->instance = teams[ i ].typeCounts[ index ];
			teams[ i ].typeCounts[ index ]++;
		}

		for ( int32_t j = 0; j < teams[ i ].capturedCount; ++j )
		{
			const pieceType_t type = GetPiece( teams[ i ].captured[ j ] )->type;
			const int32_t index = static_cast<int32_t>( type );
			teams[ i ].captureTypeCounts[ index ]++;
		}
	}
}