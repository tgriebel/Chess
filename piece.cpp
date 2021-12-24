#include "piece.h"
#include "chess.h"

bool Piece::IsValidAction( const int actionNum ) const {
	return ( actionNum >= 0 ) && ( actionNum < GetActionCount() );
}

void Piece::Move( const int targetX, const int targetY ) {
	state->SetEnpassant( NoPiece );
	if ( state->GetInfo( targetX, targetY ).team != team ) {
		state->CapturePiece( team, state->GetPiece( targetX, targetY ) );
	}

	Set( targetX, targetY );
	++moveCount;
}

void Piece::Set( const int targetX, const int targetY ) {
	if ( state->OnBoard( x, y ) ) {
		state->SetHandle( NoPiece, x, y );
	}
	if ( state->OnBoard( targetX, targetY ) ) {
		state->SetHandle( handle, targetX, targetY );
	}
	x = targetX;
	y = targetY;
}

void Piece::CalculateStep( const int actionNum, int& actionX, int& actionY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return;
	}
	actionX += GetActions()[ actionNum ].x;
	actionY += GetActions()[ actionNum ].y;
}

int Piece::GetStepCount( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return BoardSize;
	}
	if ( state->OnBoard( targetX, targetY ) == false ) {
		return BoardSize;
	}
	if ( state->GetInfo( targetX, targetY ).team == team ) {
		return BoardSize;
	}
	int nextX = x;
	int nextY = y;
	int prevDist = INT_MAX;
	int dist = INT_MAX;
	const int maxSteps = GetActions()[ actionNum ].maxSteps;
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
		if ( state->GetPiece( nextX, nextY ) != nullptr ) {
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
	return ( stepCount <= GetActions()[ actionNum ].maxSteps );
}

int Piece::GetActionPath( const int actionNum, moveAction_t path[ BoardSize ] ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return 0;
	}
	int validSquares = 0;
	const int actionCount = GetActionCount();
	int nextX = x;
	int nextY = y;
	const int maxSteps = GetActions()[ actionNum ].maxSteps;
	for ( int step = 1; step <= maxSteps; ++step ) {
		CalculateStep( actionNum, nextX, nextY );
		if ( state->IsLegalMove( this, nextX, nextY ) ) {
			path[ validSquares++ ] = moveAction_t( nextX, nextY, GetAction( actionNum ).type, 1 );
		}
	}
	return validSquares;
}

bool Pawn::InActionPath( const int actionNum, const int targetX, const int targetY ) const {
	if ( IsValidAction( actionNum ) == false ) {
		return false;
	}
	const teamCode_t occupiedTeam = state->GetInfo( targetX, targetY ).team;
	const bool isOccupied = ( occupiedTeam != teamCode_t::NONE );
	const int maxSteps = GetAction( actionNum ).maxSteps;
	const int steps = GetStepCount( actionNum, targetX, targetY );
	const moveType_t type = GetAction( actionNum ).type;

	if ( type == moveType_t::PAWN_T2X ) {
		return ( isOccupied == false ) && ( steps <= maxSteps ) && ( HasMoved() == false );
	}
	if ( ( type == moveType_t::PAWN_KILL_L ) || ( type == moveType_t::PAWN_KILL_R ) ) {
		const bool wasEnpassant = ( state->GetEnpassant( targetX, targetY ) != NoPiece );
		const bool isEnemy = ( isOccupied || wasEnpassant ) && ( occupiedTeam != team );
		return isEnemy && ( steps <= maxSteps );
	}
	return ( isOccupied == false ) && ( steps <= maxSteps );
}

bool Pawn::CanPromote() const {
	int nextX = x;
	int nextY = y;
	CalculateStep( GetActionNum( moveType_t::PAWN_T ), nextX, nextY );
	return ( state->OnBoard( nextX, nextY ) == false );
}

void Pawn::Move( const int targetX, const int targetY ) {
	const bool doubleMove = ( abs( targetY - y ) == 2 );
	const pieceHandle_t pieceHdl = state->GetEnpassant( targetX, targetY );
	Piece* targetPiece = state->GetPiece( pieceHdl );
	if ( ( targetPiece != nullptr ) && ( targetPiece->team != team ) ) {
		state->CapturePiece( team, targetPiece );
	}
	Piece::Move( targetX, targetY );
	if ( doubleMove ) {
		state->SetEnpassant( handle );
	} else {
		state->SetEnpassant( NoPiece );
	}
	if ( CanPromote() ) {
		state->PromotePawn( handle );
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
	const moveType_t type = GetAction( actionNum ).type;
	if ( type == moveType_t::KING_CASTLE_L ) {
		rook = state->GetPiece( 0, y );
	} else if ( type == moveType_t::KING_CASTLE_R ) {
		rook = state->GetPiece( BoardSize - 1, y );
	} else {
		return true;
	}
	if ( ( rook == nullptr ) || ( rook->type != pieceType_t::ROOK ) ) {
		return false;
	}
	if ( HasMoved() || rook->HasMoved() ) {
		return false;
	}
	const bool rightCastle = ( type == moveType_t::KING_CASTLE_R );
	const int flankOffset = rightCastle ? -1 : 1;
	const moveType_t moveTest = rightCastle ? moveType_t::ROOK_L : moveType_t::ROOK_R;
	const int rookTargetX = targetX + flankOffset;
	const bool rookMove = rook->InActionPath( rook->GetActionNum( moveTest ), rookTargetX, y );
	if ( rookMove == false ) {
		return false;
	}
	if ( state->GetPiece( rookTargetX, y ) != nullptr ) {
		return false;
	}
	rook->Set( rookTargetX, y );

	return true;
}