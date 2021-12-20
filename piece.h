#pragma once
#include "chess.h"
#include <limits>

class ChessBoard;

class Piece {
protected:
	static const int MaxActions = 8;

	Piece() {
		team = teamCode_t::UNASSIGNED;
		type = pieceType_t::NONE;
		x = 0;
		y = 0;
		instance = 0;
		numActions = 0;
		moveCount = 0;
		captured = false;
		handle = NoPiece;
		board = nullptr;
	}

	bool IsValidAction( const int actionNum ) const;
	void Move( const int targetX, const int targetY );

public:
	moveType_t GetMoveType( const int actionNum ) const;
	bool IsLocatedAt( const int actionX, const int actionY ) const;
	void CalculateStep( const int actionNum, int& actionX, int& actionY ) const;
	int GetStepCount( const int actionNum, const int targetX, const int targetY, const int maxSteps ) const;
	virtual bool InActionPath( const int actionNum, const int targetX, const int targetY ) const = 0;

	bool HasMoved() const {
		return ( moveCount > 0 );
	}
	int GetActionCount() const {
		return numActions;
	}
private:
	void BindBoard( const ChessBoard* board, const pieceHandle_t handle ) {
		this->board = board;
		this->handle = handle;
	}
public:
	teamCode_t			team;
	pieceType_t			type;
	int					x;
	int					y;
	int					instance;
protected:
	int					moveCount;
	int					numActions;
	bool				captured;
	moveAction_t		actions[ MaxActions ];
	pieceHandle_t		handle;
	const ChessBoard* board;

	friend class ChessBoard;
};

class Pawn : public Piece {
public:
	Pawn( const teamCode_t team, const int instance ) {
		this->type = pieceType_t::PAWN;
		this->team = team;
		this->instance = instance;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( 0, 1, PAWN_T );
		actions[ numActions++ ] = moveAction_t( 0, 2, PAWN_T2X );
		actions[ numActions++ ] = moveAction_t( 1, -1, PAWN_KILL_L );
		actions[ numActions++ ] = moveAction_t( 1, 1, PAWN_KILL_R );
	}

	bool InActionPath( const int actionNum, const int targetX, const int targetY ) const override;
};

class Rook : public Piece {
public:
	Rook( const teamCode_t team, const int instance ) {
		this->type = pieceType_t::ROOK;
		this->team = team;
		this->instance = instance;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( 0, -1, ROOK_T );
		actions[ numActions++ ] = moveAction_t( 0, 1, ROOK_B );
		actions[ numActions++ ] = moveAction_t( 1, 0, ROOK_R );
		actions[ numActions++ ] = moveAction_t( -1, 0, ROOK_L );
	}
	bool InActionPath( const int actionNum, const int targetX, const int targetY ) const override;
};

class Knight : public Piece {
public:
	Knight( const teamCode_t team, const int instance ) {
		this->type = pieceType_t::KNIGHT;
		this->team = team;
		this->instance = instance;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( -2, -1, KNIGHT_T1L2 );
		actions[ numActions++ ] = moveAction_t( -1, -2, KNIGHT_T2L1 );
		actions[ numActions++ ] = moveAction_t( 1, -2, KNIGHT_T1R2 );
		actions[ numActions++ ] = moveAction_t( 2, -1, KNIGHT_T2R1 );
		actions[ numActions++ ] = moveAction_t( 2, 1, KNIGHT_B1R2 );
		actions[ numActions++ ] = moveAction_t( 1, 2, KNIGHT_B2R1 );
		actions[ numActions++ ] = moveAction_t( -1, 2, KNIGHT_B2L1 );
		actions[ numActions++ ] = moveAction_t( -2, 1, KNIGHT_B1L2 );
	}
	bool InActionPath( const int actionNum, const int actionX, const int actionY ) const override;
};

class Bishop : public Piece {
public:
	Bishop( const teamCode_t team, const int instance ) {
		this->type = pieceType_t::BISHOP;
		this->team = team;
		this->instance = instance;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( -1, -1, BISHOP_TL );
		actions[ numActions++ ] = moveAction_t( 1, -1, BISHOP_TR );
		actions[ numActions++ ] = moveAction_t( 1, 1, BISHOP_BR );
		actions[ numActions++ ] = moveAction_t( -1, 1, BISHOP_BL );
	}
	bool InActionPath( const int actionNum, const int targetX, const int targetY ) const override;
};

class King : public Piece {
public:
	King( const teamCode_t team, const int instance ) {
		this->type = pieceType_t::KING;
		this->team = team;
		this->instance = instance;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( -1, -1, KING_TL );
		actions[ numActions++ ] = moveAction_t( 0, -1, KING_T );
		actions[ numActions++ ] = moveAction_t( 1, -1, KING_TR );
		actions[ numActions++ ] = moveAction_t( 1, 0, KING_R );
		actions[ numActions++ ] = moveAction_t( 1, 1, KING_BR );
		actions[ numActions++ ] = moveAction_t( 0, 1, KING_B );
		actions[ numActions++ ] = moveAction_t( -1, 1, KING_BL );
		actions[ numActions++ ] = moveAction_t( -1, 0, KING_L );
	}
	bool InActionPath( const int actionNum, const int actionX, const int actionY ) const override;
};

class Queen : public Piece {
public:
	Queen( const teamCode_t team, const int instance ) {
		this->type = pieceType_t::QUEEN;
		this->team = team;
		this->instance = instance;

		numActions = 0;
		actions[ numActions++ ] = moveAction_t( -1, -1, QUEEN_TL );
		actions[ numActions++ ] = moveAction_t( 0, -1, QUEEN_T );
		actions[ numActions++ ] = moveAction_t( 1, -1, QUEEN_TR );
		actions[ numActions++ ] = moveAction_t( 1, 0, QUEEN_R );
		actions[ numActions++ ] = moveAction_t( 1, 1, QUEEN_BR );
		actions[ numActions++ ] = moveAction_t( 0, 1, QUEEN_B );
		actions[ numActions++ ] = moveAction_t( -1, 1, QUEEN_BL );
		actions[ numActions++ ] = moveAction_t( -1, 0, QUEEN_L );
	}
	bool InActionPath( const int actionNum, const int targetX, const int targetY ) const override;
};