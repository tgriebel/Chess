#pragma once
#pragma once
#include "common.h"
#include "piece.h"

typedef void ( *callback_t )( callbackEvent_t& );

class Chess;
class ChessState {
public:
	ChessState() {}

	ChessState( const ChessState& state ) {
		CopyState( state );
	}

	~ChessState() {

	}

	bool				IsLegalMove( const Piece* piece, const int targetX, const int targetY ) const;
	inline bool			OnBoard( const int x, const int y ) const;
	inline const Piece* GetPiece( const pieceHandle_t handle ) const;
	inline Piece*		GetPiece( const pieceHandle_t handle );
	const Piece*		GetPiece( const int x, const int y ) const;
	Piece*				GetPiece( const int x, const int y );
	pieceInfo_t			GetInfo( const int x, const int y ) const;

	bool				IsOpenToAttackAt( const Piece* targetPiece, const int targetX, const int targetY ) const;
	pieceHandle_t		GetEnpassant( const int targetX, const int targetY ) const;
	inline void			SetEnpassant( const pieceHandle_t handle ) { enpassantPawn = handle; }
	void				PromotePawn( const pieceHandle_t pieceHdl );

private:
	pieceHandle_t		GetHandle( const int x, const int y ) const;
	void				CopyState( const ChessState& state );
	void				CapturePiece( const teamCode_t attacker, Piece* targetPiece );
	bool				FindCheckMate( const teamCode_t team );
	void				CountTeamPieces();
private:
	callback_t			callback;
	pieceHandle_t		enpassantPawn;
	Piece*				pieces[ PieceCount ];
	team_t				teams[ TeamCount ];
	pieceHandle_t		grid[ BoardSize ][ BoardSize ]; // (0,0) is top left
	Chess*				game;

	friend class Piece;
	friend class Pawn;
	friend class Chess;
};