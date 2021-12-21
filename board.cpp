#include "board.h"
#include "piece.h"

moveType_t ChessBoard::IsLegalMove( const Piece* piece, const int targetX, const int targetY ) const {
	// Check piece actions
	const int actionCount = piece->GetActionCount();
	for ( int action = 0; action < actionCount; ++action ) {
		if ( piece->InActionPath( action, targetX, targetY ) ) {
			return static_cast< moveType_t >( action );
		}
	}
	return moveType_t::NONE;
}

void ChessBoard::CapturePiece( const teamCode_t attacker, const int x, const int y ) {
	const pieceHandle_t pieceHdl = GetHandle( x, y );
	Piece* piece = GetPiece( pieceHdl );
	if ( piece == nullptr ) {
		return;
	}
	piece->RemoveFromPlay();

	const int index = static_cast<int>( piece->team );
	const int attackerIndex = static_cast<int>( attacker );
	int& capturedCount = teams[ attackerIndex ].capturedCount;
	int& playCount = teams[ index ].livingCount;

	teams[ attackerIndex ].captured[ capturedCount ] = pieceHdl;
	++capturedCount;

	for ( int i = 0; i < playCount; ++i ) {
		if ( teams[ index ].pieces[ i ] == pieceHdl ) {
			teams[ index ].pieces[ i ] = teams[ index ].pieces[ playCount - 1 ];
			--playCount;
		}
	}
}

bool ChessBoard::IsOpenToAttackAt( const pieceHandle_t pieceHdl, const int x, const int y ) const {
	const Piece* targetPiece = GetPiece( pieceHdl );
	const teamCode_t opposingTeam = ( targetPiece->team == teamCode_t::WHITE ) ? teamCode_t::BLACK : teamCode_t::WHITE;
	const int index = static_cast<int>( opposingTeam );
	for ( int i = 0; i < teams[ index ].livingCount; ++i ) {
		const Piece* piece = GetPiece( teams[ index ].pieces[ i ] );
		const int actionCount = piece->GetActionCount();
		for ( int action = 0; action < actionCount; ++action ) {
			if ( piece->InActionPath( action, x, y ) ) {
				return true;
			}
		}
	}
	return false;
}

bool ChessBoard::ForcedCheckMate( const teamCode_t team ) const {
	const pieceHandle_t kingHdl = FindPiece( team, pieceType_t::KING, 0 );
	const Piece* king = GetPiece( kingHdl );
	const int actionCount = king->GetActionCount();
	for ( int action = 0; action < actionCount; ++action ) {
		int nextX = king->x;
		int nextY = king->y;
		king->CalculateStep( action, nextX, nextY );
		if ( IsOpenToAttackAt( kingHdl, nextX, nextY ) == false ) {
			return false;
		}
	}
	return true;
}

void ChessBoard::CountTeamPieces() {
	for ( int i = 0; i < TeamCount; ++i ) {
		for ( int j = 0; j < static_cast<int>( pieceType_t::COUNT ); ++j ) {
			teams[ i ].typeCounts[ j ] = 0;
		}
		for ( int j = 0; j < teams[ i ].livingCount; ++j ) {
			Piece* piece = GetPiece( teams[ i ].pieces[ j ] );
			const pieceType_t type = piece->type;
			const int index = static_cast<int>( type );

			piece->instance = teams[ i ].typeCounts[ index ];
			teams[ i ].typeCounts[ index ]++;
		}
		for ( int j = 0; j < teams[ i ].capturedCount; ++j ) {
			const pieceType_t type = GetPiece( teams[ i ].captured[ j ] )->type;
			const int index = static_cast<int>( type );
			teams[ i ].captureTypeCounts[ index ]++;
		}
	}
}

bool ChessBoard::CanPromotePawn( const Pawn* pawn ) const {
	int nextX = pawn->x;
	int nextY = pawn->y;
	pawn->CalculateStep( 0, nextX, nextY );
	return ( IsOnBoard( nextX, nextY ) == false );
}

void ChessBoard::PromotePawn( const pieceHandle_t pieceHdl ) {
	const Piece* piece = GetPiece( pieceHdl );
	if ( ( piece == nullptr ) || ( piece->type != pieceType_t::PAWN ) ) {
		return;
	}
	const teamCode_t team = piece->team;
	const int x = piece->x;
	const int y = piece->y;

	delete pieces[ pieceHdl ];
	pieces[ pieceHdl ] = new Queen( team );
	pieces[ pieceHdl ]->BindBoard( this, pieceHdl );
	MovePiece( pieces[ pieceHdl ], x, y );
}

void ChessBoard::MovePiece( Piece* piece, const int targetX, const int targetY ) {
	grid[ piece->y ][ piece->x ] = NoPiece;
	grid[ targetY ][ targetX ] = piece->handle;
	piece->Move( targetX, targetY );
}

bool ChessBoard::PerformMoveAction( const pieceHandle_t pieceHdl, const int targetX, const int targetY ) {
	Piece* piece = GetPiece( pieceHdl );
	if ( piece == nullptr ) {
		return false;
	}
	const moveType_t legalMove = IsLegalMove( piece, targetX, targetY );
	if ( legalMove == moveType_t::NONE ) {
		return false;
	}
	if ( GetTeam( targetX, targetY ) != piece->team ) {
		CapturePiece( piece->team, targetX, targetY );
	}
	MovePiece( piece, targetX, targetY );

	if ( piece->type == pieceType_t::PAWN ) {
		if ( CanPromotePawn( reinterpret_cast<Pawn*>( piece ) ) ) {
			PromotePawn( pieceHdl );
		}
	}

	CountTeamPieces();
	return true;
}

pieceHandle_t ChessBoard::FindPiece( const teamCode_t team, const pieceType_t type, const int instance ) {
	if ( ( team == teamCode_t::NONE ) || ( type == pieceType_t::NONE ) ) {
		return NoPiece;
	}
	for ( int i = 0; i < pieceNum; ++i ) {
		const bool teamsMatch = ( pieces[ i ]->team == team );
		const bool piecesMatch = ( pieces[ i ]->type == type );
		const bool instanceMatch = ( pieces[ i ]->instance == instance );
		if ( teamsMatch && piecesMatch && instanceMatch ) {
			return i;
		}
	}
	return NoPiece;
}

bool ChessBoard::IsValidHandle( const pieceHandle_t handle ) const {
	if ( handle == NoPiece ) {
		return false;
	}
	if ( ( handle < 0 ) && ( handle >= pieceNum ) ) {
		return false;
	}
	return true;
}

pieceHandle_t ChessBoard::GetHandle( const int x, const int y ) const {
	if ( IsOnBoard( x, y ) == false ) {
		return OffBoard;
	}
	return grid[ y ][ x ];
}