#include "Chess.h"

MoveCache PawnMoveSuperset;
MoveCache RookMoveSuperset;
MoveCache KnightMoveSuperset;
MoveCache BishopMoveSuperset;
MoveCache KingMoveSuperset;
MoveCache QueenMoveSuperset;

bool Piece::IsValidAction( const int32_t actionNum ) const
{
	return ( actionNum >= 0 ) && ( actionNum < GetActionCount() );
}


void Piece::Move( const moveType_t moveType, const num_t targetX, const num_t targetY )
{
	state->SetEnpassant( NoPiece );

	if ( state->IsBlocked( team, targetX, targetY ) == false )
	{
		Piece* opponentPiece = state->GetPiece( targetX, targetY );

		state->CapturePiece( team, opponentPiece );
	}

	PlaceAt( targetX, targetY );
	++moveCount;
}


void Piece::PlaceAt( const num_t targetX, const num_t targetY )
{
	if ( state->OnBoard( x, y ) ) {
		state->SetHandle( NoPiece, x, y );
	}

	if ( state->OnBoard( targetX, targetY ) ) {
		state->SetHandle( handle, targetX, targetY );
	}

	x = targetX;
	y = targetY;
}


void Piece::TempPlacement( const num_t targetX, const num_t targetY )
{
	assert( ( x != -1 ) && ( y != -1 ) );

	prevX = x;
	prevY = y;

	PlaceAt( targetX, targetY );
}


void Piece::ReturnPlacement()
{
	PlaceAt( prevX, prevY );

	prevX = -1;
	prevY = -1;
}


void Piece::CalculateStep( const int32_t actionNum, num_t& actionX, num_t& actionY ) const
{
	assert( IsValidAction( actionNum ) );

	const moveAction_t& action = GetAction( actionNum );
	actionX += action.x;
	actionY += action.y * GetTeamDirection();
}


num_t Piece::GetStepCount( const int32_t actionNum, const num_t targetX, const num_t targetY ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return BoardSize;
	}

	if ( state->OnBoard( targetX, targetY ) == false ) {
		return BoardSize;
	}

	if ( state->IsBlocked( team, targetX, targetY ) ) {
		return BoardSize;
	}

	num_t nextX = x;
	num_t nextY = y;
	num_t prevDist = INT8_MAX;
	num_t dist = INT8_MAX;

	const int32_t maxSteps = GetActions()[ actionNum ].maxSteps;

	for ( int32_t step = 1; step <= maxSteps; ++step )
	{
		CalculateStep( actionNum, nextX, nextY );

		prevDist = dist;
		dist = abs( targetX - nextX ) + abs( targetY - nextY );

		if ( dist >= prevDist ) {
			return BoardSize;
		}

		if ( dist == 0 ) {
			return step;
		}

		if ( state->GetPiece( nextX, nextY ) != nullptr ) {
			return BoardSize;
		}
	}
	return BoardSize;
}


bool Piece::InActionPath( const int32_t actionNum, const num_t targetX, const num_t targetY ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const int32_t stepCount = GetStepCount( actionNum, targetX, targetY );
	return ( stepCount <= GetActions()[ actionNum ].maxSteps );
}


num_t Piece::GetActionPath( const int32_t actionNum, moveAction_t path[ BoardSize ] ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return 0;
	}

	num_t validSquares = 0;
	const int32_t actionCount = GetActionCount();
	num_t nextX = x;
	num_t nextY = y;
	const int32_t maxSteps = GetActions()[ actionNum ].maxSteps;

	for ( int32_t step = 1; step <= maxSteps; ++step )
	{
		CalculateStep( actionNum, nextX, nextY );

		if ( state->IsLegalMove( this, nextX, nextY ) != moveType_t::NONE ) {
			path[ validSquares++ ] = moveAction_t( nextX, nextY, GetAction( actionNum ).type, 1 );
		}
	}
	return validSquares;
}


void Piece::FillMoveCache()
{
	MoveCache& superset = *const_cast<MoveCache*>( moveSuperset );

	// Only fill once per piece type (shared static)
	if ( superset.bits[ 0 ] != 0 || superset.bits[ 1 ] != 0 ||
		 superset.bits[ 2 ] != 0 || superset.bits[ 3 ] != 0 ) {
		return;
	}

	const int32_t actionCount = GetActionCount();

	for ( int32_t action = 0; action < actionCount; ++action )
	{
		num_t nextX = 0;
		num_t nextY = 0;

		const int32_t maxSteps = GetActions()[ action ].maxSteps;

		for ( int32_t step = 1; step <= maxSteps; ++step )
		{
			CalculateStep( action, nextX, nextY );
			superset.Set( nextX, nextY );
		}
	}
}


bool Pawn::InActionPath( const int32_t actionNum, const num_t targetX, const num_t targetY ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}

	const bool isOccupied = state->GetHandle( targetX, targetY ) != NoPiece;
	const bool isBlocked = state->IsBlocked( team, targetX, targetY );

	const num_t maxSteps = GetAction( actionNum ).maxSteps;
	const num_t steps = GetStepCount( actionNum, targetX, targetY );

	const moveType_t type = GetAction( actionNum ).type;

	if ( type == moveType_t::PAWN_T2X ) {
		return ( isOccupied == false ) && ( steps <= maxSteps ) && ( HasMoved() == false );
	}

	if ( ( type == moveType_t::PAWN_KILL_L ) || ( type == moveType_t::PAWN_KILL_R ) )
	{
		const Piece* enpassantPiece = state->GetEnpassant( targetX, targetY );
		const bool isEnpassantEnemy = ( enpassantPiece != nullptr ) && ( enpassantPiece->team != team );
		const bool isEnemy = ( isOccupied || isEnpassantEnemy ) && ( isBlocked == false );

		return isEnemy && ( steps <= maxSteps );
	}

	return ( isOccupied == false ) && ( steps <= maxSteps );
}


bool Pawn::CanPromote() const
{
	num_t nextX = x;
	num_t nextY = y;
	CalculateStep( GetActionNum( moveType_t::PAWN_T ), nextX, nextY );

	return ( state->OnBoard( nextX, nextY ) == false );
}


void Pawn::Move( const moveType_t moveType, const num_t targetX, const num_t targetY )
{
	const bool doubleMove = ( abs( targetY - y ) == 2 );
	Piece* targetPiece = state->GetEnpassant( targetX, targetY );

	if ( ( targetPiece != nullptr ) && ( targetPiece->team != team ) ) {
		state->CapturePiece( team, targetPiece );
	}

	Piece::Move( moveType, targetX, targetY );

	if ( doubleMove ) {
		state->SetEnpassant( handle );
	} else {
		state->SetEnpassant( NoPiece );
	}

	if ( CanPromote() ) {
		Promote();
	}
}


void Pawn::Promote()
{
	callbackEvent_t event;
	event.type = PAWN_PROMOTION;
	event.promotionType = pieceType_t::NONE;

	state->PromotionCallback( team, event );

	bool invalidChoice = true;
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::QUEEN );
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::KNIGHT );
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::BISHOP );
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::ROOK );

	if ( invalidChoice ) {
		event.promotionType = pieceType_t::QUEEN;
	}

	type = event.promotionType;

	switch( type )
	{
		case pieceType_t::ROOK:
		{
			actions = RookActions;
			moveSuperset = &RookMoveSuperset;
			numActions = static_cast<int32_t>( moveType_t::ROOK_ACTIONS );
		} break;

		case pieceType_t::BISHOP:
		{
			actions = BishopActions;
			moveSuperset = &BishopMoveSuperset;
			numActions = static_cast<int32_t>( moveType_t::BISHOP_ACTIONS );
		} break;

		case pieceType_t::KNIGHT:
		{
			actions = KnightActions;
			moveSuperset = &KnightMoveSuperset;
			numActions = static_cast<int32_t>( moveType_t::KNIGHT_ACTIONS );
		} break;

		case pieceType_t::QUEEN:
		{
			actions = QueenActions;
			moveSuperset = &QueenMoveSuperset;
			numActions = static_cast<int32_t>( moveType_t::QUEEN_ACTIONS );
		} break;
	}

	promoted = true;
}


bool King::InActionPath( const int32_t actionNum, const num_t targetX, const num_t targetY ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}

	const num_t stepCount = GetStepCount( actionNum, targetX, targetY );
	if ( stepCount != 1 ) {
		return false;
	}

	Piece* castlePiece = nullptr;
	const moveType_t type = GetAction( actionNum ).type;

	if ( type == moveType_t::KING_CASTLE_L ) {
		castlePiece = state->GetPiece( 0, y );
	} else if ( type == moveType_t::KING_CASTLE_R ) {
		castlePiece = state->GetPiece( BoardSize - 1, y );
	} else {
		return true;
	}

	if ( ( castlePiece == nullptr ) || ( castlePiece->type != pieceType_t::ROOK ) ) {
		return false;
	}

	if ( HasMoved() || castlePiece->HasMoved() ) {
		return false;
	}

	const bool rightCastle = ( type == moveType_t::KING_CASTLE_R );
	const num_t flankOffset = rightCastle ? -1 : 1;
	const moveType_t moveTest = rightCastle ? moveType_t::ROOK_L : moveType_t::ROOK_R;

	const num_t rookTargetX = targetX + flankOffset;
	const bool rookMove = castlePiece->InActionPath( castlePiece->GetActionNum( moveTest ), rookTargetX, y ); // Cheaks a clear path between rook and king

	if ( rookMove == false ) {
		return false;
	}

	if ( state->GetPiece( rookTargetX, y ) != nullptr ) {
		return false;
	}

	// Illegal: Castle while in check
	// Illegal: Castle through any attacked square
	// Illegal: Castle into checked square (covered by general rule)
	if ( state->IsChecked( team ) || state->IsOpenToAttackAt( castlePiece, rightCastle ? x + 1 : x - 1, y ) ) {
		return false;
	}

	return true;
}


void King::Move( const moveType_t moveType, const num_t targetX, const num_t targetY )
{
	const bool isCastleAction = ( moveType == moveType_t::KING_CASTLE_L ) || ( moveType == moveType_t::KING_CASTLE_R );

	if( isCastleAction == false )
	{
		Piece::Move( moveType, targetX, targetY );
		return;
	}

	Piece* rook = nullptr;
	if( moveType == moveType_t::KING_CASTLE_L )
	{
		rook = state->GetPiece( 0, y );

		assert( rook && rook->type == pieceType_t::ROOK ); // Already tested legal

		rook->PlaceAt( targetX + 1, y );
	}
	else if( moveType == moveType_t::KING_CASTLE_R )
	{
		rook = state->GetPiece( BoardSize - 1, y );

		assert( rook && rook->type == pieceType_t::ROOK ); // Already tested legal

		rook->PlaceAt( targetX - 1, y );
	}
	PlaceAt( targetX, targetY );
}