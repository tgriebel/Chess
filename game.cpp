#include "game.h"
#include "piece.h"

#include <iostream>

bool ChessState::IsLegalMove( const Piece* piece, const int targetX, const int targetY ) const {
	if ( OnBoard( targetX, targetY ) == false ) {
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
	//// It's illegal for any move to leave that team's king checked
	//const Piece* movedPiece = GetPiece( targetX, targetY );
	//const int x = piece->x;
	//const int y = piece->y;
	//const_cast<Piece*>( piece )->Set( targetX, targetY ); // FIXME: hack
	//const pieceHandle_t kingHdl = FindPiece( piece->team, pieceType_t::KING, 0 );
	//const Piece* king = GetPiece( kingHdl );
	//if ( IsOpenToAttackAt( king, king->x, king->y ) ) {
	//	isLegal = false;
	//}
	//const_cast<Piece*>( piece )->Set( x, y ); // FIXME: hack
	//if ( movedPiece != nullptr ) {
	//	const_cast<Piece*>( movedPiece )->Set( targetX, targetY ); // FIXME: hack
	//}
	return isLegal;
}

void ChessState::CapturePiece( const teamCode_t attacker, Piece* targetPiece ) {
	if ( targetPiece == nullptr ) {
		return;
	}
	targetPiece->RemoveFromPlay();

	const int index = static_cast<int>( targetPiece->team );
	const int attackerIndex = static_cast<int>( attacker );
	int& capturedCount =teams[ attackerIndex ].capturedCount;
	int& playCount = teams[ index ].livingCount;

	teams[ attackerIndex ].captured[ capturedCount ] = targetPiece->handle;
	++capturedCount;

	for ( int i = 0; i < playCount; ++i ) {
		if ( teams[ index ].pieces[ i ] == targetPiece->handle ) {
			teams[ index ].pieces[ i ] = teams[ index ].pieces[ playCount - 1 ];
			--playCount;
		}
	}
	return;
}

bool ChessState::IsOpenToAttackAt( const Piece* targetPiece, const int x, const int y ) const {
	if ( OnBoard( x, y ) == false ) {
		return false;
	}
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

bool ChessState::FindCheckMate( const teamCode_t team ) {
	//const pieceHandle_t kingHdl = FindPiece( team, pieceType_t::KING, 0 );
	//const Piece* king = GetPiece( kingHdl );
	//// King was captured
	//if ( king == nullptr ) {			
	//	return true;
	//}
	//// King can't move
	//const int actionCount = king->GetActionCount();
	//for ( int action = 0; action < actionCount; ++action ) {
	//	int nextX = king->x;
	//	int nextY = king->y;
	//	king->CalculateStep( action, nextX, nextY );
	//	if( IsOpenToAttackAt( king, nextX, nextY ) == false ) {
	//		return false;
	//	}
	//}
	return true;
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

void ChessState::PromotePawn( const pieceHandle_t pieceHdl ) {
	const Piece* piece = GetPiece( pieceHdl );
	if ( ( piece == nullptr ) || ( piece->type != pieceType_t::PAWN ) ) {
		return;
	}

	callbackEvent_t event;
	event.type = PAWN_PROMOTION;
	event.promotionType = pieceType_t::NONE;
	
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

bool Chess::PerformMoveAction( const pieceHandle_t pieceHdl, const int targetX, const int targetY ) {
	Piece* piece = s->GetPiece( pieceHdl );
	if ( piece == nullptr ) {
		return false;
	}
	const bool legalMove = s->IsLegalMove( piece, targetX, targetY );
	if ( legalMove == false ) {
		return false;
	}
	piece->Move( targetX, targetY );

	const teamCode_t opposingTeam = GetOpposingTeam( piece->team );
	if ( s->FindCheckMate( opposingTeam ) ) {
		winner = piece->team;
	}
	s->CountTeamPieces();
	return true;
}

pieceHandle_t Chess::FindPiece( const teamCode_t team, const pieceType_t type, const int instance ) {
	if ( ( team == teamCode_t::NONE ) || ( type == pieceType_t::NONE ) ) {
		return NoPiece;
	}
	for ( int i = 0; i < s->pieceNum; ++i ) {
		const bool teamsMatch = ( s->pieces[ i ]->team == team );
		const bool piecesMatch = ( s->pieces[ i ]->type == type );
		const bool instanceMatch = ( s->pieces[ i ]->instance == instance );
		if ( teamsMatch && piecesMatch && instanceMatch ) {
			return i;
		}
	}
	return NoPiece;
}

bool ChessState::IsValidHandle( const pieceHandle_t handle ) const {
	if ( handle == NoPiece ) {
		return false;
	}
	if ( ( handle < 0 ) && ( handle >= pieceNum ) ) {
		return false;
	}
	return true;
}

Piece* Chess::CreatePiece( const pieceType_t pieceType, const teamCode_t teamCode ) {
	switch ( pieceType ) {
	case pieceType_t::PAWN:		return new Pawn( teamCode );
	case pieceType_t::ROOK:		return new Rook( teamCode );
	case pieceType_t::KNIGHT:	return new Knight( teamCode );
	case pieceType_t::BISHOP:	return new Bishop( teamCode );
	case pieceType_t::QUEEN:	return new Queen( teamCode );
	case pieceType_t::KING:		return new King( teamCode );
	}
	return nullptr;
}

pieceHandle_t ChessState::GetHandle( const int x, const int y ) const {
	if ( OnBoard( x, y ) == false ) {
		return OffBoard;
	}
	return grid[ y ][ x ];
}