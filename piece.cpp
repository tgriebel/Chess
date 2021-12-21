#include "piece.h"
#include "board.h"

bool Piece::IsValidAction( const int actionNum ) const {
	return ( actionNum >= 0 ) && ( actionNum < GetActionCount() );
}

void Piece::Move( const int targetX, const int targetY ) {
	this->x = targetX;
	this->y = targetY;
	++moveCount;
}

moveType_t Piece::GetMoveType( const int actionNum ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return moveType_t::NONE;
	}
	return actions[ actionNum ].type;
}

bool Piece::IsLocatedAt( const int actionX, const int actionY ) const {
	return ( x == actionX ) && ( y == actionY );
}

void Piece::CalculateStep( const int actionNum, int& actionX, int& actionY ) const {
	actionX += actions[ actionNum ].x;
	actionY += actions[ actionNum ].y;
}

int Piece::GetStepCount( const int actionNum, const int targetX, const int targetY, const int maxSteps ) const {
	if ( board->GetTeam( targetX, targetY ) == team ) {
		return BoardSize;
	}
	int nextX = x;
	int nextY = y;
	int prevDist = INT_MAX;
	int dist = INT_MAX;
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

bool Pawn::InActionPath( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const teamCode_t occupiedTeam = board->GetTeam( targetX, targetY );
	const bool isOccupied = ( occupiedTeam != teamCode_t::NONE );
	const int steps = GetStepCount( actionNum, targetX, targetY, 1 );
	const moveType_t type = actions[ actionNum ].type;
	if ( type == PAWN_T2X ) {
		return ( isOccupied == false ) && ( steps == 1 ) && ( HasMoved() == false );
	}
	if ( ( type == PAWN_KILL_L ) || ( type == PAWN_KILL_R ) ) {
		const bool isEnemy = isOccupied && ( occupiedTeam != team );
		return isEnemy && ( steps == 1 );
	}
	return ( isOccupied == false ) && ( steps == 1 );
}

bool Rook::InActionPath( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const int stepCount = GetStepCount( actionNum, targetX, targetY, BoardSize );
	return ( stepCount < BoardSize );
}

bool Knight::InActionPath( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const int stepCount = GetStepCount( actionNum, targetX, targetY, 1 );
	return ( stepCount == 1 );
}

bool Bishop::InActionPath( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const int stepCount = GetStepCount( actionNum, targetX, targetY, BoardSize );
	return ( stepCount < BoardSize );
}

bool King::InActionPath( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	if ( board->IsOpenToAttackAt( handle, targetX, targetY ) == false ) {
		return false;
	}
	const int stepCount = GetStepCount( actionNum, targetX, targetY, 1 );
	return ( stepCount == 1 );
}

bool Queen::InActionPath( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const int stepCount = GetStepCount( actionNum, targetX, targetY, BoardSize );
	return ( stepCount < BoardSize );
}