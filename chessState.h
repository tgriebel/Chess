#pragma once
#pragma once
#include "chess.h"
#include "piece.h"

typedef void ( *callback_t )( callbackEvent_t& );

class Chess;
class ChessState {
public:
	ChessState(/* const ChessState& baseState */) {
		
	}

	~ChessState() {

	}

	bool IsLegalMove( const Piece* piece, const int targetX, const int targetY ) const;

	teamCode_t GetTeam( const int x, const int y ) const {
		return game->GetInfo( x, y ).team;
	}

	inline bool OnBoard( const int x, const int y ) const {
		return ( x >= 0 ) && ( x < BoardSize ) && ( y >= 0 ) && ( y < BoardSize );
	}

	inline const Piece* GetPiece( const pieceHandle_t handle ) const {
		return const_cast<ChessState*>( this )->GetPiece( handle );
	}

	inline Piece* GetPiece( const pieceHandle_t handle );

	inline const Piece* GetPiece( const int x, const int y ) const {
		return const_cast<ChessState*>( this )->GetPiece( x, y );
	}

	inline Piece* GetPiece( const int x, const int y );

	bool IsOpenToAttackAt( const Piece* targetPiece, const int targetX, const int targetY ) const;
	void SetEnpassant( const pieceHandle_t handle ) {
		enpassantPawn = handle;
	}
	pieceHandle_t GetEnpassant( const int targetX, const int targetY ) const {
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

	void PromotePawn( const pieceHandle_t pieceHdl );

private:
	pieceHandle_t GetHandle( const int x, const int y ) const;
	void CapturePiece( const teamCode_t attacker, Piece* targetPiece );
	bool FindCheckMate( const teamCode_t team );
	void CountTeamPieces();
private:
	callback_t		callback;
	pieceHandle_t	enpassantPawn;
	Piece*			pieces[ PieceCount ];
	team_t			teams[ TeamCount ];
	pieceHandle_t	grid[ BoardSize ][ BoardSize ]; // (0,0) is top left
	Chess*			game;

	friend class Piece;
	friend class Pawn;
	friend class Chess;
};