#pragma once
#include "chess.h"
#include <limits>

class ChessState;

class Piece {
protected:
	static const int MaxActions = 10;

	Piece() {
		team = teamCode_t::NONE;
		type = pieceType_t::NONE;
		x = -1;
		y = -1;
		numActions = 0;
		moveCount = 0;
		instance = 0;
		handle = NoPiece;
		board = nullptr;
	}

	bool IsValidAction( const int actionNum ) const;
public:
	moveType_t GetMoveType( const int actionNum ) const;
	void CalculateStep( const int actionNum, int& actionX, int& actionY ) const;
	int GetStepCount( const int actionNum, const int targetX, const int targetY ) const;
	void EnumerateActions( std::vector< moveAction_t >& actions ) const;
	virtual bool InActionPath( const int actionNum, const int targetX, const int targetY ) const;
	virtual void Move( const int targetX, const int targetY );
	void Set( const int targetX, const int targetY );

	bool HasMoved() const {
		return ( moveCount > 0 );
	}
	int GetActionCount() const {
		return numActions;
	}
	void RemoveFromPlay() {
		Set( -1, -1 );
		board = nullptr;
	}
	bool OnBoard() const {
		return ( board != nullptr );
	}
	int GetActionNum( const moveType_t moveType ) const {
		for ( int i = 0; i < numActions; ++i ) {
			if ( actions->type == moveType ) {
				return i;
			}
		}
		return -1;
	}
private:
	void BindBoard( ChessState* board, const pieceHandle_t handle ) {
		this->board = board;
		this->handle = handle;
	}
public:
	teamCode_t			team;
	pieceType_t			type;
	int					instance;
protected:
	int					x;
	int					y;
	int					moveCount;
	int					numActions;
	moveAction_t		actions[ MaxActions ];
	pieceHandle_t		handle;

	ChessState*			board;

	friend class Chess;
	friend class ChessState;
};

class Pawn : public Piece {
public:
	Pawn( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::PAWN;
		this->team = team;

		const int direction = GetDirection();

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( 0, direction * 1, PAWN_T, 1 );
		actions[ numActions++ ] = moveAction_t( 0, direction * 2, PAWN_T2X, 1 );
		actions[ numActions++ ] = moveAction_t( -1, direction * 1, PAWN_KILL_L, 1 );
		actions[ numActions++ ] = moveAction_t( 1, direction * 1, PAWN_KILL_R, 1 );
		assert( numActions <= MaxActions );
	}

	bool InActionPath( const int actionNum, const int targetX, const int targetY ) const override;
	void Move( const int targetX, const int targetY ) override;
	bool CanPromote() const;

	inline int GetDirection() const {
		return ( team == teamCode_t::WHITE ) ? -1 : 1;
	}
};

class Rook : public Piece {
public:
	Rook( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::ROOK;
		this->team = team;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( 0, 1, ROOK_T, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( 0, -1, ROOK_B, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( 1, 0, ROOK_R, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( -1, 0, ROOK_L, BoardSize - 1 );
		assert( numActions <= MaxActions );
	}
};

class Knight : public Piece {
public:
	Knight( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::KNIGHT;
		this->team = team;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( -2, -1, KNIGHT_T1L2, 1 );
		actions[ numActions++ ] = moveAction_t( -1, -2, KNIGHT_T2L1, 1 );
		actions[ numActions++ ] = moveAction_t( 1, -2, KNIGHT_T1R2, 1 );
		actions[ numActions++ ] = moveAction_t( 2, -1, KNIGHT_T2R1, 1 );
		actions[ numActions++ ] = moveAction_t( 2, 1, KNIGHT_B1R2, 1 );
		actions[ numActions++ ] = moveAction_t( 1, 2, KNIGHT_B2R1, 1 );
		actions[ numActions++ ] = moveAction_t( -1, 2, KNIGHT_B2L1, 1 );
		actions[ numActions++ ] = moveAction_t( -2, 1, KNIGHT_B1L2, 1 );
		assert( numActions <= MaxActions );
	}
};

class Bishop : public Piece {
public:
	Bishop( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::BISHOP;
		this->team = team;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( -1, -1, BISHOP_TL, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( 1, -1, BISHOP_TR, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( 1, 1, BISHOP_BR, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( -1, 1, BISHOP_BL, BoardSize - 1 );
		assert( numActions <= MaxActions );
	}
};

class King : public Piece {
public:
	King( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::KING;
		this->team = team;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( -1, -1, KING_TL, 1 );
		actions[ numActions++ ] = moveAction_t( 0, -1, KING_T, 1 );
		actions[ numActions++ ] = moveAction_t( 1, -1, KING_TR, 1 );
		actions[ numActions++ ] = moveAction_t( 1, 0, KING_R, 1 );
		actions[ numActions++ ] = moveAction_t( 1, 1, KING_BR, 1 );
		actions[ numActions++ ] = moveAction_t( 0, 1, KING_B, 1 );
		actions[ numActions++ ] = moveAction_t( -1, 1, KING_BL, 1 );
		actions[ numActions++ ] = moveAction_t( -1, 0, KING_L, 1 );
		actions[ numActions++ ] = moveAction_t( -2, 0, KING_CASTLE_L, 1 );
		actions[ numActions++ ] = moveAction_t( 2, 0, KING_CASTLE_R, 1 );
		assert( numActions <= MaxActions );
	}
	bool InActionPath( const int actionNum, const int actionX, const int actionY ) const override;
};

class Queen : public Piece {
public:
	Queen( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::QUEEN;
		this->team = team;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( -1, -1, QUEEN_TL, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( 0, -1, QUEEN_T, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( 1, -1, QUEEN_TR, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( 1, 0, QUEEN_R, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( 1, 1, QUEEN_BR, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( 0, 1, QUEEN_B, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( -1, 1, QUEEN_BL, BoardSize - 1 );
		actions[ numActions++ ] = moveAction_t( -1, 0, QUEEN_L, BoardSize - 1 );
		assert( numActions <= MaxActions );
	}
};