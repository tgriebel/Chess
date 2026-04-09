#include "Chess.h"

MoveCache PawnMoveSuperset;
MoveCache RookMoveSuperset;
MoveCache KnightMoveSuperset;
MoveCache BishopMoveSuperset;
MoveCache KingMoveSuperset;
MoveCache QueenMoveSuperset;


static const int32_t PieceValue[ (int32_t)pieceType_t::COUNT ] =
{
	100,	// PAWN
	500,	// ROOK
	320,	// KNIGHT
	330,	// BISHOP
	20000,	// KING
	900,	// QUEEN
};


// Pawn PST — encourages central advancement
static const int32_t PawnPST[ BoardSize ][ BoardSize ] =
{
	{  0,  0,  0,  0,  0,  0,  0,  0 },	// rank 8 (promotion rank for white)
	{ 50, 50, 50, 50, 50, 50, 50, 50 },	// rank 7
	{ 10, 10, 20, 30, 30, 20, 10, 10 },	// rank 6
	{  5,  5, 10, 25, 25, 10,  5,  5 },	// rank 5
	{  0,  0,  0, 20, 20,  0,  0,  0 },	// rank 4
	{  5, -5,-10,  0,  0,-10, -5,  5 },	// rank 3
	{  5, 10, 10,-20,-20, 10, 10,  5 },	// rank 2
	{  0,  0,  0,  0,  0,  0,  0,  0 },	// rank 1 (white pawn start)
};


// Knight PST — strong in the center, weak on edges
static const int32_t KnightPST[ BoardSize ][ BoardSize ] =
{
	{ -50,-40,-30,-30,-30,-30,-40,-50 },
	{ -40,-20,  0,  0,  0,  0,-20,-40 },
	{ -30,  0, 10, 15, 15, 10,  0,-30 },
	{ -30,  5, 15, 20, 20, 15,  5,-30 },
	{ -30,  0, 15, 20, 20, 15,  0,-30 },
	{ -30,  5, 10, 15, 15, 10,  5,-30 },
	{ -40,-20,  0,  5,  5,  0,-20,-40 },
	{ -50,-40,-30,-30,-30,-30,-40,-50 },
};


// Bishop PST — prefers long diagonals, avoids edges
static const int32_t BishopPST[ BoardSize ][ BoardSize ] =
{
	{ -20,-10,-10,-10,-10,-10,-10,-20 },
	{ -10,  0,  0,  0,  0,  0,  0,-10 },
	{ -10,  0,  5, 10, 10,  5,  0,-10 },
	{ -10,  5,  5, 10, 10,  5,  5,-10 },
	{ -10,  0, 10, 10, 10, 10,  0,-10 },
	{ -10, 10, 10, 10, 10, 10, 10,-10 },
	{ -10,  5,  0,  0,  0,  0,  5,-10 },
	{ -20,-10,-10,-10,-10,-10,-10,-20 },
};


// Rook PST — 7th rank is strong, open files
static const int32_t RookPST[ BoardSize ][ BoardSize ] =
{
	{  0,  0,  0,  0,  0,  0,  0,  0 },
	{  5, 10, 10, 10, 10, 10, 10,  5 },
	{ -5,  0,  0,  0,  0,  0,  0, -5 },
	{ -5,  0,  0,  0,  0,  0,  0, -5 },
	{ -5,  0,  0,  0,  0,  0,  0, -5 },
	{ -5,  0,  0,  0,  0,  0,  0, -5 },
	{ -5,  0,  0,  0,  0,  0,  0, -5 },
	{  0,  0,  0,  5,  5,  0,  0,  0 },
};


// Queen PST — light positional guidance, mostly material-driven
static const int32_t QueenPST[ BoardSize ][ BoardSize ] =
{
	{ -20,-10,-10, -5, -5,-10,-10,-20 },
	{ -10,  0,  0,  0,  0,  0,  0,-10 },
	{ -10,  0,  5,  5,  5,  5,  0,-10 },
	{  -5,  0,  5,  5,  5,  5,  0, -5 },
	{   0,  0,  5,  5,  5,  5,  0, -5 },
	{ -10,  5,  5,  5,  5,  5,  0,-10 },
	{ -10,  0,  5,  0,  0,  0,  0,-10 },
	{ -20,-10,-10, -5, -5,-10,-10,-20 },
};


// King PST (middlegame) — stay castled and sheltered
static const int32_t KingPST[ BoardSize ][ BoardSize ] =
{
	{ -30,-40,-40,-50,-50,-40,-40,-30 },
	{ -30,-40,-40,-50,-50,-40,-40,-30 },
	{ -30,-40,-40,-50,-50,-40,-40,-30 },
	{ -30,-40,-40,-50,-50,-40,-40,-30 },
	{ -20,-30,-30,-40,-40,-30,-30,-20 },
	{ -10,-20,-20,-20,-20,-20,-20,-10 },
	{  20, 20,  0,  0,  0,  0, 20, 20 },
	{  20, 30, 10,  0,  0, 10, 30, 20 },
};


static const int32_t( *PST[ (int32_t)pieceType_t::COUNT ] )[ BoardSize ] =
{
	PawnPST,	// PAWN   = 0
	RookPST,	// ROOK   = 1
	KnightPST,	// KNIGHT = 2
	BishopPST,	// BISHOP = 3
	KingPST,	// KING   = 4
	QueenPST,	// QUEEN  = 5
};


bool Piece::IsValidAction( const int32_t actionNum ) const
{
	return ( actionNum >= 0 ) && ( actionNum < GetActionCount() );
}


void Piece::Move( const moveType_t moveType, const num_t targetX, const num_t targetY )
{
	const bool isCastleAction = ( moveType == moveType_t::KING_CASTLE_L ) || ( moveType == moveType_t::KING_CASTLE_R );
	const bool isPawnAction = ( type == pieceType_t::PAWN );

	if( isCastleAction )
	{
		Piece* rook = nullptr;
		if ( moveType == moveType_t::KING_CASTLE_L )
		{
			rook = m_state->GetPiece( 0, m_y );

			assert( rook && rook->type == pieceType_t::ROOK ); // Already tested legal

			rook->PlaceAt( targetX + 1, m_y );
		}
		else if ( moveType == moveType_t::KING_CASTLE_R )
		{
			rook = m_state->GetPiece( BoardSize - 1, m_y );

			assert( rook && rook->type == pieceType_t::ROOK ); // Already tested legal

			rook->PlaceAt( targetX - 1, m_y );
		}
		PlaceAt( targetX, targetY );
	}
	else if( isPawnAction )
	{
		const bool doubleMove = ( abs( targetY - m_y ) == 2 );
		Piece* targetPiece = m_state->GetEnpassant( targetX, targetY );

		if ( targetPiece == nullptr ) {
			targetPiece = m_state->GetPiece( targetX, targetY );
		}
		
		if ( m_state->IsBlocked( team, targetX, targetY ) == false ) {
			m_state->CapturePiece( team, targetPiece );
		}

		PlaceAt( targetX, targetY );
		++m_moveCount;

		m_state->SetEnpassant( doubleMove ? m_handle : NoPiece );

		if ( CanPromote() ) {
			Promote();
		}
	}
	else
	{
		m_state->SetEnpassant( NoPiece );

		if ( m_state->IsBlocked( team, targetX, targetY ) == false )
		{
			Piece* opponentPiece = m_state->GetPiece( targetX, targetY );

			m_state->CapturePiece( team, opponentPiece );
		}

		PlaceAt( targetX, targetY );
		++m_moveCount;
	}
}


void Piece::PlaceAt( const num_t targetX, const num_t targetY )
{
	if ( m_state->OnBoard( m_x, m_y ) ) {
		m_state->SetHandle( NoPiece, m_x, m_y );
	}

	if ( m_state->OnBoard( targetX, targetY ) ) {
		m_state->SetHandle( m_handle, targetX, targetY );
	}

	m_x = targetX;
	m_y = targetY;
}


void Piece::TempPlacement( const num_t targetX, const num_t targetY )
{
	assert( ( m_x != -1 ) && ( m_y != -1 ) );

	m_prevX = m_x;
	m_prevY = m_y;

	PlaceAt( targetX, targetY );
}


void Piece::ReturnPlacement()
{
	PlaceAt( m_prevX, m_prevY );

	m_prevX = -1;
	m_prevY = -1;
}



bool Piece::CanPromote() const
{
	if( type != pieceType_t::PAWN ) {
		return false;
	}

	num_t nextX = m_x;
	num_t nextY = m_y;
	CalculateStep( GetActionNum( moveType_t::PAWN_T ), nextX, nextY );

	return ( m_state->OnBoard( nextX, nextY ) == false );
}


void Piece::Promote()
{
	if ( type != pieceType_t::PAWN ) {
		return;
	}

	callbackEvent_t event;
	event.type = PAWN_PROMOTION;
	event.promotionType = pieceType_t::NONE;

	m_state->PromotionCallback( team, event );

	bool invalidChoice = true;
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::QUEEN );
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::KNIGHT );
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::BISHOP );
	invalidChoice = invalidChoice && ( event.promotionType != pieceType_t::ROOK );

	if ( invalidChoice ) {
		event.promotionType = pieceType_t::QUEEN;
	}

	type = event.promotionType;

	switch ( type )
	{
	case pieceType_t::ROOK:
	{
		m_actions = RookActions;
		m_moveSuperset = &RookMoveSuperset;
		m_numActions = static_cast<int32_t>( moveType_t::ROOK_ACTIONS );
	} break;

	case pieceType_t::BISHOP:
	{
		m_actions = BishopActions;
		m_moveSuperset = &BishopMoveSuperset;
		m_numActions = static_cast<int32_t>( moveType_t::BISHOP_ACTIONS );
	} break;

	case pieceType_t::KNIGHT:
	{
		m_actions = KnightActions;
		m_moveSuperset = &KnightMoveSuperset;
		m_numActions = static_cast<int32_t>( moveType_t::KNIGHT_ACTIONS );
	} break;

	case pieceType_t::QUEEN:
	{
		m_actions = QueenActions;
		m_moveSuperset = &QueenMoveSuperset;
		m_numActions = static_cast<int32_t>( moveType_t::QUEEN_ACTIONS );
	} break;
	}

	m_promoted = true;
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

	if ( m_state->OnBoard( targetX, targetY ) == false ) {
		return BoardSize;
	}

	if ( m_state->IsBlocked( team, targetX, targetY ) ) {
		return BoardSize;
	}

	num_t nextX = m_x;
	num_t nextY = m_y;
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

		if ( m_state->GetPiece( nextX, nextY ) != nullptr ) {
			return BoardSize;
		}
	}
	return BoardSize;
}


bool Piece::IsPawnMoveValid( const int32_t actionNum, const num_t targetX, const num_t targetY ) const
{
	// Assume these have already been checked, and we're only checking additional pawn-specific conditions
	assert( IsValidAction( actionNum ) );
	assert ( m_state->OnBoard( targetX, targetY ) );

	const moveType_t actionType = GetAction( actionNum ).type;

	const bool isOccupied = m_state->GetHandle( targetX, targetY ) != NoPiece;
	const bool isBlocked = m_state->IsBlocked( team, targetX, targetY );

	const num_t maxSteps = GetAction( actionNum ).maxSteps;
	const num_t steps = GetStepCount( actionNum, targetX, targetY );

	const moveType_t type = GetAction( actionNum ).type;

	if ( actionType == moveType_t::PAWN_T2X ) {
		return ( isOccupied == false ) && ( steps <= maxSteps ) && ( HasMoved() == false );
	}

	if ( ( actionType == moveType_t::PAWN_KILL_L ) || ( actionType == moveType_t::PAWN_KILL_R ) )
	{
		const Piece* enpassantPiece = m_state->GetEnpassant( targetX, targetY );
		const bool isEnpassantEnemy = ( enpassantPiece != nullptr ) && ( enpassantPiece->team != team );
		const bool isEnemy = ( isOccupied || isEnpassantEnemy ) && ( isBlocked == false );

		return isEnemy && ( steps <= maxSteps );
	}

	return ( isOccupied == false ) && ( steps <= maxSteps );
}


bool Piece::IsKingMoveValid( const int32_t actionNum, const num_t targetX, const num_t targetY ) const
{
	Piece* castlePiece = nullptr;
	const moveType_t actionType = GetAction( actionNum ).type;

	if ( actionType == moveType_t::KING_CASTLE_L ) {
		castlePiece = m_state->GetPiece( 0, m_y );
	}
	else if ( actionType == moveType_t::KING_CASTLE_R ) {
		castlePiece = m_state->GetPiece( BoardSize - 1, m_y );
	}
	else {
		return true;
	}

	if ( ( castlePiece == nullptr ) || ( castlePiece->type != pieceType_t::ROOK ) ) {
		return false;
	}

	if ( HasMoved() || castlePiece->HasMoved() ) {
		return false;
	}

	const bool rightCastle = ( actionType == moveType_t::KING_CASTLE_R );
	const num_t flankOffset = rightCastle ? -1 : 1;
	const moveType_t moveTest = rightCastle ? moveType_t::ROOK_L : moveType_t::ROOK_R;

	const num_t rookTargetX = targetX + flankOffset;
	const bool rookMove = castlePiece->InActionPath( castlePiece->GetActionNum( moveTest ), rookTargetX, m_y ); // Cheaks a clear path between rook and king

	if ( rookMove == false ) {
		return false;
	}

	if ( m_state->GetPiece( rookTargetX, m_y ) != nullptr ) {
		return false;
	}

	// Illegal: Castle while in check
	// Illegal: Castle through any attacked square
	// Illegal: Castle into checked square (covered by general rule)
	if ( m_state->IsChecked( team ) || m_state->IsOpenToAttackAt( castlePiece, m_x - flankOffset, m_y ) ) {
		return false;
	}
	return true;
}


int32_t Piece::ComputeAllMoveActions( position_t* moves ) const
{
	const int32_t actionCount = GetActionCount();

	int32_t moveCounter = 0;

	for ( int32_t actionId = 0; actionId < actionCount; ++actionId )
	{
		moveCounter += ComputeActionPath( actionId, moves + moveCounter );
	}
	return moveCounter;
}


int32_t Piece::ComputeActionPath( const int32_t actionNum, position_t* path ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return 0;
	}

	const moveType_t actionType = GetAction( actionNum ).type;

	num_t nextX = m_x;
	num_t nextY = m_y;

	const int32_t maxSteps = GetActions()[ actionNum ].maxSteps;

	int32_t step = 0;

	while ( step < maxSteps )
	{
		CalculateStep( actionNum, nextX, nextY );

		if ( m_state->OnBoard( nextX, nextY ) == false ) {
			return step;
		}

		if ( m_state->IsBlocked( team, nextX, nextY ) ) {
			break;
		}

		if ( type == pieceType_t::PAWN )
		{
			// Enpassant, double-move, kills
			if( IsPawnMoveValid( actionNum, nextX, nextY ) == false ) {
				break;
			}
		}
		else if ( type == pieceType_t::KING )
		{
			// Castling
			if ( IsKingMoveValid( actionNum, nextX, nextY ) == false ) {
				break;
			}
		}

		path[ step ] = position_t{ nextX, nextY };
		++step;

		// Capture square
		if ( m_state->GetPiece( nextX, nextY ) != nullptr ) {
			break;
		}
	}
	return step;
}


bool Piece::InActionPath( const int32_t actionNum, const num_t targetX, const num_t targetY ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}

	// Using more generalized code, old code left for testing during transition
	// Old code uses a distance heuristic to early out, embedden in `GetStepCount`
#if 1
	position_t path[ BoardSize ] = {};	
	int32_t stepCount = ComputeActionPath( actionNum, path );

	int32_t step = 0;

	for ( int32_t step = 0; step < stepCount; ++step )
	{
		if( ( path[ step ].x == targetX ) && ( path[ step ].y == targetY ) )
		{
			return true;
		}
	}
	return false;
#else
	const int32_t stepCount = GetStepCount( actionNum, targetX, targetY );
	if( stepCount > GetActions()[ actionNum ].maxSteps ){
		return false;
	}

	// Need additional checks
	if( type == pieceType_t::PAWN  )
	{
		return IsPawnMoveValid( actionNum, targetX, targetY );
	}
	else if( type == pieceType_t::KING )
	{
		return IsKingMoveValid( actionNum, targetX, targetY );
	}
	return true;
#endif
}


num_t Piece::GetActionPath( const int32_t actionNum, moveAction_t path[ BoardSize ] ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return 0;
	}

	num_t validSquares = 0;
	const int32_t actionCount = GetActionCount();
	num_t nextX = m_x;
	num_t nextY = m_y;
	const int32_t maxSteps = GetActions()[ actionNum ].maxSteps;

	for ( int32_t step = 1; step <= maxSteps; ++step )
	{
		CalculateStep( actionNum, nextX, nextY );

		if ( m_state->IsLegalMove( this, nextX, nextY ) != moveType_t::NONE ) {
			path[ validSquares++ ] = moveAction_t( nextX, nextY, GetAction( actionNum ).type, 1 );
		}
	}
	return validSquares;
}


void Piece::FillMoveCache()
{
	MoveCache& superset = *const_cast<MoveCache*>( m_moveSuperset );

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