#include "Chess.h"


bool Piece::IsValidAction( const int32_t actionNum ) const
{
	return ( actionNum >= 0 ) && ( actionNum < GetActionCount() );
}


void Piece::Move( const moveType_t moveType, const int32_t targetX, const int32_t targetY )
{
	state->SetEnpassant( NoPiece );

	const pieceInfo_t& targetInfo = state->GetInfo( targetX, targetY );

	// Is this square occupied by an an opponent piece?
	// Blank squares have a NONE assignment
	if ( targetInfo.onBoard && ( targetInfo.team != team ) )
	{
		Piece* opponentPiece = state->GetPiece( targetX, targetY );

		state->CapturePiece( team, opponentPiece );
	}

	PlaceAt( targetX, targetY );
	++moveCount;
}


void Piece::PlaceAt( const int32_t targetX, const int32_t targetY )
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


void Piece::TempPlacement( const int32_t targetX, const int32_t targetY )
{
	assert( ( x != -1 ) && ( y != -1 ) );

	prevX = x;
	prevY = y;

	PlaceAt( targetX, targetY );
}


void Piece::ReturnPlacement()
{
	assert( ( prevX != -1 ) && ( prevY != -1 ) );

	PlaceAt( prevX, prevY );

	prevX = -1;
	prevY = -1;
}


void Piece::CalculateStep( const int32_t actionNum, int32_t& actionX, int32_t& actionY ) const
{
	assert( IsValidAction( actionNum ) );

	const moveAction_t& action = GetAction( actionNum );
	actionX += action.x;
	actionY += action.y * GetTeamDirection();
}


int32_t Piece::GetStepCount( const int32_t actionNum, const int32_t targetX, const int32_t targetY ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return BoardSize;
	}

	if ( state->OnBoard( targetX, targetY ) == false ) {
		return BoardSize;
	}

	if ( state->GetInfo( targetX, targetY ).team == team ) {
		return BoardSize;
	}

	int32_t nextX = x;
	int32_t nextY = y;
	int32_t prevDist = INT_MAX;
	int32_t dist = INT_MAX;

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


bool Piece::InActionPath( const int32_t actionNum, const int32_t targetX, const int32_t targetY ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const int32_t stepCount = GetStepCount( actionNum, targetX, targetY );
	return ( stepCount <= GetActions()[ actionNum ].maxSteps );
}


int32_t Piece::GetActionPath( const int32_t actionNum, moveAction_t path[ BoardSize ] ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return 0;
	}

	int32_t validSquares = 0;
	const int32_t actionCount = GetActionCount();
	int32_t nextX = x;
	int32_t nextY = y;
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


bool Pawn::InActionPath( const int32_t actionNum, const int32_t targetX, const int32_t targetY ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}

	const teamCode_t occupiedTeam = state->GetInfo( targetX, targetY ).team;
	const bool isOccupied = ( occupiedTeam != teamCode_t::NONE );

	const int32_t maxSteps = GetAction( actionNum ).maxSteps;
	const int32_t steps = GetStepCount( actionNum, targetX, targetY );

	const moveType_t type = GetAction( actionNum ).type;

	if ( type == moveType_t::PAWN_T2X ) {
		return ( isOccupied == false ) && ( steps <= maxSteps ) && ( HasMoved() == false );
	}

	if ( ( type == moveType_t::PAWN_KILL_L ) || ( type == moveType_t::PAWN_KILL_R ) )
	{
		const pieceHandle_t enpassantPieceHdl = state->GetEnpassant( targetX, targetY );
		const Piece* enpassantPiece = state->GetPiece( enpassantPieceHdl );
		const bool isEnpassantEnemy = ( enpassantPieceHdl != NoPiece ) && ( enpassantPiece->team != team );
		const bool isEnemy = ( isOccupied || isEnpassantEnemy ) && ( occupiedTeam != team );

		return isEnemy && ( steps <= maxSteps );
	}

	return ( isOccupied == false ) && ( steps <= maxSteps );
}


bool Pawn::CanPromote() const
{
	int32_t nextX = x;
	int32_t nextY = y;
	CalculateStep( GetActionNum( moveType_t::PAWN_T ), nextX, nextY );

	return ( state->OnBoard( nextX, nextY ) == false );
}


void Pawn::Move( const moveType_t moveType, const int32_t targetX, const int32_t targetY )
{
	const bool doubleMove = ( abs( targetY - y ) == 2 );
	const pieceHandle_t pieceHdl = state->GetEnpassant( targetX, targetY );

	Piece* targetPiece = state->GetPiece( pieceHdl );

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
		state->PromotePawn( handle );
	}
}


bool King::InActionPath( const int32_t actionNum, const int32_t targetX, const int32_t targetY ) const
{
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}

	const int32_t stepCount = GetStepCount( actionNum, targetX, targetY );
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
	const int32_t flankOffset = rightCastle ? -1 : 1;
	const moveType_t moveTest = rightCastle ? moveType_t::ROOK_L : moveType_t::ROOK_R;

	const int32_t rookTargetX = targetX + flankOffset;
	const bool rookMove = castlePiece->InActionPath( castlePiece->GetActionNum( moveTest ), rookTargetX, y );

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


void King::Move( const moveType_t moveType, const int32_t targetX, const int32_t targetY )
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