#include "board.h"
#include "piece.h"

bool ChessBoard::IsLegalMove( const Piece* piece, const int targetX, const int targetY ) const {
	// Check piece actions
	const int actionCount = piece->GetActionCount();
	for ( int action = 0; action < actionCount; ++action ) {
		if ( piece->InActionPath( action, targetX, targetY ) ) {
			return true;
		}
	}
	return false;
}

void ChessBoard::CapturePiece( const int x, const int y ) {
	const pieceHandle_t pieceHdl = GetHandle( x, y );
	Piece* piece = GetPiece( pieceHdl );
	if ( piece == nullptr ) {
		return;
	}
	piece->captured = true;
	grid[ y ][ x ] = NoPiece;

	const int index = static_cast<int>( piece->team );
	int& capturedCount = piecesCaptured[ index ];
	int& playCount = piecesOnBoard[ index ];

	captured[ index ][ capturedCount ] = pieceHdl;
	++capturedCount;

	for ( int i = 0; i < TeamPieceCount; ++i ) {
		if ( team[ index ][ i ] == pieceHdl ) {
			team[ index ][ i ] = team[ index ][ playCount - 1 ];
			--playCount;
		}
	}
}

bool ChessBoard::MovePiece( const pieceHandle_t pieceHdl, const int targetX, const int targetY ) {
	Piece* piece = GetPiece( pieceHdl );
	if ( piece == nullptr ) {
		return false;
	}
	const bool legalMove = IsLegalMove( piece, targetX, targetY );
	if ( legalMove == false ) {
		return false;
	}
	if ( IsOccupied( targetX, targetY ) ) {
		CapturePiece( targetX, targetY );
	}
	grid[ piece->y ][ piece->x ] = NoPiece;
	grid[ targetY ][ targetX ] = pieceHdl;
	piece->Move( targetX, targetY );

	return true;
}

pieceHandle_t ChessBoard::FindPiece( const teamCode_t team, const pieceType_t type, const int instance ) {
	for ( int i = 0; i < PieceCount; ++i ) {
		const bool teamsMatch = ( pieces[ i ]->team == team );
		const bool piecesMatch = ( pieces[ i ]->type == type );
		const bool instanceMatch = ( pieces[ i ]->instance == instance );
		if ( teamsMatch && piecesMatch && instanceMatch ) {
			return i;
		}
	}
	return NoPiece;
}

void ChessBoard::GetPieceLocation( const pieceHandle_t handle, int& x, int& y ) const {
	for ( int i = 0; i < BoardSize; ++i ) {
		for ( int j = 0; j < BoardSize; ++j ) {
			if ( grid[ i ][ j ] != handle ) {
				x = j;
				y = i;
				return;
			}
		}
	}
}

bool ChessBoard::IsValidHandle( const pieceHandle_t handle ) const {
	if ( handle == NoPiece ) {
		return false;
	}
	if ( ( handle < 0 ) && ( handle >= PieceCount ) ) {
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