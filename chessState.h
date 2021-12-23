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

	void SetBoard( const gameConfig_t& cfg );

	void EnterPieceInGame( Piece* piece, const int x, const int y ) {
		pieces[ pieceNum ] = piece;
		pieces[ pieceNum ]->BindBoard( this, pieceNum );
		pieces[ pieceNum ]->Set( x, y );

		const int teamIndex = static_cast<int>( piece->team );
		const int pieceIndex = teams[ teamIndex ].livingCount;
		teams[ teamIndex ].pieces[ pieceIndex ] = pieceNum;
		++teams[ teamIndex ].livingCount;
		++pieceNum;
	}

	bool IsLegalMove( const Piece* piece, const int targetX, const int targetY ) const;

	teamCode_t GetTeam( const int x, const int y ) const {
		if ( OnBoard( x, y ) == false ) {
			return teamCode_t::NONE;
		}
		const Piece* targetPiece = GetPiece( x, y );
		const bool isOccupied = ( targetPiece != nullptr );
		return ( targetPiece != nullptr ) ? targetPiece->team : teamCode_t::NONE;
	}

	inline bool OnBoard( const int x, const int y ) const {
		return ( x >= 0 ) && ( x < BoardSize ) && ( y >= 0 ) && ( y < BoardSize );
	}

	inline const Piece* GetPiece( const pieceHandle_t handle ) const {
		return const_cast<ChessState*>( this )->GetPiece( handle );
	}

	inline Piece* GetPiece( const pieceHandle_t handle ) {
		if ( IsValidHandle( handle ) ) {
			return pieces[ handle ];
		}
		return nullptr;
	}

	inline const Piece* GetPiece( const int x, const int y ) const {
		return const_cast<ChessState*>( this )->GetPiece( x, y );
	}

	inline Piece* GetPiece( const int x, const int y ) {
		const pieceHandle_t handle = GetHandle( x, y );
		if ( IsValidHandle( handle ) == false ) {
			return nullptr;
		}
		return pieces[ handle ];
	}

	bool IsOpenToAttackAt( const Piece* targetPiece, const int targetX, const int targetY ) const;
	void SetEnpassant( const pieceHandle_t handle ) {
		enpassantPawn = handle;
	}
	Piece* GetEnpassant( const int targetX, const int targetY ) {
		Piece* piece = GetPiece( enpassantPawn );
		if ( piece != nullptr ) {
			const Pawn* pawn = reinterpret_cast<const Pawn*>( piece );
			const int x = pawn->x;
			const int y = ( pawn->y - pawn->GetDirection() );
			const bool wasEnpassant = ( x == targetX ) && ( y == targetY );
			if ( wasEnpassant ) {
				return piece;
			}
		}
		return nullptr;
	}

	void PromotePawn( const pieceHandle_t pieceHdl );

private:
	bool IsValidHandle( const pieceHandle_t handle ) const;
	pieceHandle_t GetHandle( const int x, const int y ) const;
	void CapturePiece( const teamCode_t attacker, Piece* targetPiece );
	bool FindCheckMate( const teamCode_t team );
	void CountTeamPieces();
private:
	callback_t		callback;
	pieceHandle_t	enpassantPawn;
	int				pieceNum;
	Piece*			pieces[ PieceCount ];
	team_t			teams[ TeamCount ];
	pieceHandle_t	grid[ BoardSize ][ BoardSize ]; // (0,0) is top left

	friend class Piece;
	friend class Pawn;
	friend class Chess;
};