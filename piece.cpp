#include "piece.h"
#include "game.h"

bool Piece::IsValidAction( const int actionNum ) const {
	return ( actionNum >= 0 ) && ( actionNum < GetActionCount() );
}

void Piece::Move( const int targetX, const int targetY ) {
	board->SetEnpassant( NoPiece );
	if ( board->GetInfo( targetX, targetY ).team != team ) {
		board->CapturePiece( team, board->GetPiece( targetX, targetY ) );
	}

	Set( targetX, targetY );
	++moveCount;
}

void Piece::Set( const int targetX, const int targetY ) {
	if ( board->OnBoard( x, y ) ) {
		board->grid[ y ][ x ] = NoPiece;
	}
	if ( board->OnBoard( targetX, targetY ) ) {
		board->grid[ targetY ][ targetX ] = handle;
	}
	x = targetX;
	y = targetY;
}

moveType_t Piece::GetMoveType( const int actionNum ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return moveType_t::NONE;
	}
	return actions[ actionNum ].type;
}

void Piece::CalculateStep( const int actionNum, int& actionX, int& actionY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return;
	}
	actionX += actions[ actionNum ].x;
	actionY += actions[ actionNum ].y;
}

int Piece::GetStepCount( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return BoardSize;
	}
	if ( board->OnBoard( targetX, targetY ) == false ) {
		return BoardSize;
	}
	if ( board->GetInfo( targetX, targetY ).team == team ) {
		return BoardSize;
	}
	int nextX = x;
	int nextY = y;
	int prevDist = INT_MAX;
	int dist = INT_MAX;
	const int maxSteps = actions[ actionNum ].maxSteps;
	for ( int step = 1; step <= maxSteps; ++step ) {
		CalculateStep( actionNum, nextX, nextY );
		prevDist = dist;
		dist = abs( targetX - nextX ) + abs( targetY - nextY );
		if ( dist >= prevDist ) {
			return BoardSize;
		}
		if ( dist == 0 ) {
			return step;
		}
		if ( board->GetPiece( nextX, nextY ) != nullptr ) {
			return BoardSize;
		}
	}
	return BoardSize;
}

bool Piece::InActionPath( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const int stepCount = GetStepCount( actionNum, targetX, targetY );
	return ( stepCount <= actions[ actionNum ].maxSteps );
}

int Piece::GetActionPath( const int actionNum, moveAction_t path[ BoardSize ] ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return 0;
	}
	int validSquares = 0;
	const int actionCount = GetActionCount();
	int nextX = x;
	int nextY = y;
	const int maxSteps = actions[ actionNum ].maxSteps;
	for ( int step = 1; step <= maxSteps; ++step ) {
		CalculateStep( actionNum, nextX, nextY );
		if ( board->IsLegalMove( this, nextX, nextY ) ) {
			path[ validSquares++ ] = moveAction_t( nextX, nextY, GetMoveType( actionNum ), 1 );
		}
	}
	return validSquares;
}

bool Pawn::InActionPath( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const teamCode_t occupiedTeam = board->GetInfo( targetX, targetY ).team;
	const bool isOccupied = ( occupiedTeam != teamCode_t::NONE );
	const int maxSteps = actions[ actionNum ].maxSteps;
	const int steps = GetStepCount( actionNum, targetX, targetY );
	const moveType_t type = actions[ actionNum ].type;

	if ( type == PAWN_T2X ) {
		return ( isOccupied == false ) && ( steps <= maxSteps ) && ( HasMoved() == false );
	}
	if ( ( type == PAWN_KILL_L ) || ( type == PAWN_KILL_R ) ) {
		const bool wasEnpassant = ( board->GetEnpassant( targetX, targetY ) != NoPiece );
		const bool isEnemy = ( isOccupied || wasEnpassant ) && ( occupiedTeam != team );
		return isEnemy && ( steps <= maxSteps );
	}
	return ( isOccupied == false ) && ( steps <= maxSteps );
}

bool Pawn::CanPromote() const {
	int nextX = x;
	int nextY = y;
	CalculateStep( GetActionNum( PAWN_T ), nextX, nextY );
	return ( board->OnBoard( nextX, nextY ) == false );
}

void Pawn::Move( const int targetX, const int targetY ) {
	const bool doubleMove = ( abs( targetY - y ) == 2 );
	const pieceHandle_t pieceHdl = board->GetEnpassant( targetX, targetY );
	Piece* targetPiece = board->GetPiece( pieceHdl );
	if ( ( targetPiece != nullptr ) && ( targetPiece->team != team ) ) {
		board->CapturePiece( team, targetPiece );
	}
	Piece::Move( targetX, targetY );
	if ( doubleMove ) {
		board->SetEnpassant( handle );
	} else {
		board->SetEnpassant( NoPiece );
	}
	if ( CanPromote() ) {
		board->PromotePawn( handle );
	}
}

bool King::InActionPath( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const int stepCount = GetStepCount( actionNum, targetX, targetY );
	if ( stepCount != 1 ) {
		return false;
	}

	Piece* rook = nullptr;
	const moveType_t type = actions[ actionNum ].type;
	if ( type == moveType_t::KING_CASTLE_L ) {
		rook = board->GetPiece( 0, y );
	} else if ( type == moveType_t::KING_CASTLE_R ) {
		rook = board->GetPiece( BoardSize - 1, y );
	} else {
		return true;
	}
	if ( ( rook == nullptr ) || ( rook->type != pieceType_t::ROOK ) ) {
		return false;
	}
	if ( HasMoved() || rook->HasMoved() ) {
		return false;
	}
	const int flankOffset = ( targetX > x ) ? -1 : 1;
	const int rookTargetX = targetX + flankOffset;
	const bool rookMove = rook->InActionPath( rook->GetActionNum( ROOK_R ), rookTargetX, y );
	if ( rookMove == false ) {
		return false;
	}
	if ( board->GetPiece( rookTargetX, y ) != nullptr ) {
		return false;
	}
	rook->Set( rookTargetX, y );

	return true;
}