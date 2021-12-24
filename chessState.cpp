#include "chess.h"
#include "chessState.h"

pieceHandle_t ChessState::GetHandle( const int x, const int y ) const {
	if ( OnBoard( x, y ) == false ) {
		return OffBoard;
	}
	return grid[ y ][ x ];
}

bool ChessState::OnBoard( const int x, const int y ) const {
	return ( x >= 0 ) && ( x < BoardSize ) && ( y >= 0 ) && ( y < BoardSize );
}

const Piece* ChessState::GetPiece( const pieceHandle_t handle ) const {
	return const_cast<ChessState*>( this )->GetPiece( handle );
}

Piece* ChessState::GetPiece( const pieceHandle_t handle ) {
	if ( game->IsValidHandle( handle ) ) {
		return pieces[ handle ];
	}
	return nullptr;
}

const Piece* ChessState::GetPiece( const int x, const int y ) const {
	return const_cast<ChessState*>( this )->GetPiece( x, y );
}

Piece* ChessState::GetPiece( const int x, const int y ) {
	const pieceHandle_t handle = GetHandle( x, y );
	if ( game->IsValidHandle( handle ) == false ) {
		return nullptr;
	}
	return pieces[ handle ];
}

pieceInfo_t ChessState::GetInfo( const int x, const int y ) const {
	return game->GetInfo( x, y );
}

bool ChessState::IsLegalMove( const Piece* piece, const int targetX, const int targetY ) const {
	if ( OnBoard( targetX, targetY ) == false ) {
		return false;
	}
	if ( OnBoard( piece->x, piece->y ) == false ) {
		return false;
	}
	// Check piece actions
	bool isLegal = false;
	const int actionCount = piece->GetActionCount();
	for ( int action = 0; action < actionCount; ++action ) {
		if ( piece->InActionPath( action, targetX, targetY ) ) {
			isLegal = true;
			break;
		}
	}
	if ( isLegal == false ) {
		return false;
	}
	// It's illegal for any move to leave that team's king checked
	ChessState state( *this );
	Piece* movedPiece = state.GetPiece( piece->handle );
	movedPiece->Move( targetX, targetY );
	const teamCode_t opposingTeam = Chess::GetOpposingTeam( movedPiece->team );
	if ( state.FindCheckMate( opposingTeam ) ) {
		isLegal = false;
	}
	return isLegal;
}

void ChessState::CapturePiece( const teamCode_t attacker, Piece* targetPiece ) {
	if ( targetPiece == nullptr ) {
		return;
	}
	targetPiece->RemoveFromPlay();

	const int index			= static_cast<int>( targetPiece->team );
	const int attackerIndex	= static_cast<int>( attacker );
	int& capturedCount		= teams[ attackerIndex ].capturedCount;
	int& playCount			= teams[ index ].livingCount;
	pieceHandle_t* pieces	= teams[ index ].pieces;

	teams[ attackerIndex ].captured[ capturedCount ] = targetPiece->handle;
	++capturedCount;

	for ( int i = 0; i < playCount; ++i ) {
		if ( pieces[ i ] == targetPiece->handle ) {
			pieces[ i ] = pieces[ playCount - 1 ];
			--playCount;
		}
	}
	return;
}

void ChessState::PromotePawn( const pieceHandle_t pieceHdl ) {
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
	const int x = piece->x;
	const int y = piece->y;

	delete pieces[ pieceHdl ];

	pieces[ pieceHdl ] = Chess::CreatePiece( event.promotionType, team );
	pieces[ pieceHdl ]->BindBoard( this, pieceHdl );
	pieces[ pieceHdl ]->Move( x, y );
}

bool ChessState::IsOpenToAttackAt( const Piece* targetPiece, const int x, const int y ) const {
	if ( OnBoard( x, y ) == false ) {
		return false;
	}
	const teamCode_t opposingTeam = Chess::GetOpposingTeam( targetPiece->team );
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

bool ChessState::FindCheckMate( const teamCode_t team ) {
	const pieceHandle_t kingHdl = game->FindPiece( team, pieceType_t::KING, 0 );
	const Piece* king = GetPiece( kingHdl );
	// King was captured
	if ( king == nullptr ) {			
		return true;
	}
	// King can't move
	const int actionCount = king->GetActionCount();
	for ( int action = 0; action < actionCount; ++action ) {
		int nextX = king->x;
		int nextY = king->y;
		king->CalculateStep( action, nextX, nextY );
		if( IsOpenToAttackAt( king, nextX, nextY ) == false ) {
			return false;
		}
	}
	return true;
}

pieceHandle_t ChessState::GetEnpassant( const int targetX, const int targetY ) const {
	const Piece* piece = GetPiece( enpassantPawn );
	if ( piece != nullptr ) {
		const Pawn* pawn = reinterpret_cast<const Pawn*>( piece );
		const int x = pawn->x;
		const int y = ( pawn->y - pawn->GetDirection() );
		const bool wasEnpassant = ( x == targetX ) && ( y == targetY );
		if ( wasEnpassant ) {
			return piece->handle;
		}
	}
	return NoPiece;
}

void ChessState::CountTeamPieces() {
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

void ChessState::CopyState( const ChessState& state ) {
	callback = state.callback;
	game = state.game;
	enpassantPawn = state.enpassantPawn;
	for ( int i = 0; i < TeamCount; ++i ) {
		teams[ i ] = state.teams[ i ];
	}
	for ( int i = 0; i < BoardSize; ++i ) {
		for ( int j = 0; j < BoardSize; ++j ) {
			grid[ i ][ j ] = state.grid[ i ][ j ];
		}
	}
	for ( int i = 0; i < game->GetPieceCount(); ++i ) {
		const Piece* srcPiece = state.pieces[ i ];
		pieces[ i ] = Chess::CreatePiece( srcPiece->type, srcPiece->team );
		*pieces[ i ] = *state.pieces[ i ];
	}
}