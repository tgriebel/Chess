#pragma once
#include "common.h"
#include <limits>

class ChessState;

static const moveAction_t PawnActions[ (int)moveType_t::PAWN_ACTIONS ] ={
	moveAction_t( 0, 1, moveType_t::PAWN_T, 1 ),
	moveAction_t( 0, 2, moveType_t::PAWN_T2X, 1 ),
	moveAction_t( -1, 1, moveType_t::PAWN_KILL_L, 1 ),
	moveAction_t( 1, 1, moveType_t::PAWN_KILL_R, 1 )
};

static const moveAction_t RookActions[ (int)moveType_t::ROOK_ACTIONS ] = {
	moveAction_t( 0, 1, moveType_t::ROOK_T, BoardSize - 1 ),
	moveAction_t( 0, -1, moveType_t::ROOK_B, BoardSize - 1 ),
	moveAction_t( 1, 0, moveType_t::ROOK_R, BoardSize - 1 ),
	moveAction_t( -1, 0, moveType_t::ROOK_L, BoardSize - 1 )
};

static const moveAction_t KnightActions[ (int)moveType_t::KNIGHT_ACTIONS ] = {
	moveAction_t( -2, -1, moveType_t::KNIGHT_T1L2, 1 ),
	moveAction_t( -1, -2, moveType_t::KNIGHT_T2L1, 1 ),
	moveAction_t( 1, -2, moveType_t::KNIGHT_T1R2, 1 ),
	moveAction_t( 2, -1, moveType_t::KNIGHT_T2R1, 1 ),
	moveAction_t( 2, 1, moveType_t::KNIGHT_B1R2, 1 ),
	moveAction_t( 1, 2, moveType_t::KNIGHT_B2R1, 1 ),
	moveAction_t( -1, 2, moveType_t::KNIGHT_B2L1, 1 ),
	moveAction_t( -2, 1, moveType_t::KNIGHT_B1L2, 1 )
};

static const moveAction_t BishopActions[ (int)moveType_t::BISHOP_ACTIONS ] = {
	moveAction_t( -1, -1, moveType_t::BISHOP_TL, BoardSize - 1 ),
	moveAction_t( 1, -1, moveType_t::BISHOP_TR, BoardSize - 1 ),
	moveAction_t( 1, 1, moveType_t::BISHOP_BR, BoardSize - 1 ),
	moveAction_t( -1, 1, moveType_t::BISHOP_BL, BoardSize - 1 )
};

static const moveAction_t KingActions[ (int)moveType_t::KING_ACTIONS ] = {
	moveAction_t( -1, -1, moveType_t::KING_TL, 1 ),
	moveAction_t( 0, -1, moveType_t::KING_T, 1 ),
	moveAction_t( 1, -1, moveType_t::KING_TR, 1 ),
	moveAction_t( 1, 0, moveType_t::KING_R, 1 ),
	moveAction_t( 1, 1, moveType_t::KING_BR, 1 ),
	moveAction_t( 0, 1, moveType_t::KING_B, 1 ),
	moveAction_t( -1, 1, moveType_t::KING_BL, 1 ),
	moveAction_t( -1, 0, moveType_t::KING_L, 1 ),
	moveAction_t( -2, 0, moveType_t::KING_CASTLE_L, 1 ),
	moveAction_t( 2, 0, moveType_t::KING_CASTLE_R, 1 )
};

static const moveAction_t QueenActions[ (int)moveType_t::QUEEN_ACTIONS ] = {
	moveAction_t( -1, -1, moveType_t::QUEEN_TL, BoardSize - 1 ),
	moveAction_t( 0, -1, moveType_t::QUEEN_T, BoardSize - 1 ),
	moveAction_t( 1, -1, moveType_t::QUEEN_TR, BoardSize - 1 ),
	moveAction_t( 1, 0, moveType_t::QUEEN_R, BoardSize - 1 ),
	moveAction_t( 1, 1, moveType_t::QUEEN_BR, BoardSize - 1 ),
	moveAction_t( 0, 1, moveType_t::QUEEN_B, BoardSize - 1 ),
	moveAction_t( -1, 1, moveType_t::QUEEN_BL, BoardSize - 1 ),
	moveAction_t( -1, 0, moveType_t::QUEEN_L, BoardSize - 1 )
};

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
		state = nullptr;
	}

	bool			IsValidAction( const int actionNum ) const;
public:
	void			CalculateStep( const int actionNum, int& actionX, int& actionY ) const;
	int				GetStepCount( const int actionNum, const int targetX, const int targetY ) const;
	int				GetActionPath( const int actionNum, moveAction_t path[ BoardSize ] ) const;
	virtual bool	InActionPath( const int actionNum, const int targetX, const int targetY ) const;
	virtual void	Move( const int targetX, const int targetY );
	void			Set( const int targetX, const int targetY );

	bool HasMoved() const {
		return ( moveCount > 0 );
	}
	int GetActionCount() const {
		return numActions;
	}
	void RemoveFromPlay() {
		Set( -1, -1 );
		state = nullptr;
	}
	bool OnBoard() const {
		return ( state != nullptr );
	}
	int GetActionNum( const moveType_t moveType ) const {
		const moveAction_t* actions = GetActions();
		const int actionCount = GetActionCount();
		for ( int i = 0; i < actionCount; ++i ) {
			if ( actions[ i ].type == moveType ) {
				return i;
			}
		}
		return -1;
	}
	const moveAction_t& GetAction( const int actionNum ) const {
		return GetActions()[ actionNum ];
	}
	virtual const moveAction_t* GetActions() const = 0;
private:
	void BindBoard( ChessState* state, const pieceHandle_t handle ) {
		this->state = state;
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
	pieceHandle_t		handle;

	ChessState*			state;

	friend class Chess;
	friend class ChessState;
};

class Pawn : public Piece {
public:
	Pawn( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::PAWN;
		this->team = team;
		this->numActions = static_cast<int>( moveType_t::PAWN_ACTIONS );
	}

	bool InActionPath( const int actionNum, const int targetX, const int targetY ) const override;
	void Move( const int targetX, const int targetY ) override;
	bool CanPromote() const;

	inline int GetDirection() const {
		return ( team == teamCode_t::WHITE ) ? -1 : 1;
	}

	const moveAction_t* GetActions() const {
		return PawnActions;
	}
private:
	inline bool IsKillAction( const moveType_t type ) const {
		return ( type == moveType_t::PAWN_KILL_L ) || ( type == moveType_t::PAWN_KILL_R );
	}
};

class Rook : public Piece {
public:
	Rook( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::ROOK;
		this->team = team;
		this->numActions = static_cast<int>( moveType_t::ROOK_ACTIONS );
	}
	const moveAction_t* GetActions() const {
		return RookActions;
	}
};

class Knight : public Piece {
public:
	Knight( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::KNIGHT;
		this->team = team;
		this->numActions = static_cast<int>( moveType_t::KNIGHT_ACTIONS );
	}
	const moveAction_t* GetActions() const {
		return KnightActions;
	}
};

class Bishop : public Piece {
public:
	Bishop( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::BISHOP;
		this->team = team;
		this->numActions = static_cast<int>( moveType_t::BISHOP_ACTIONS );
	}
	const moveAction_t* GetActions() const {
		return BishopActions;
	}
};

class King : public Piece {
public:
	King( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::KING;
		this->team = team;
		this->numActions = static_cast<int>( moveType_t::KING_ACTIONS );
	}
	const moveAction_t* GetActions() const {
		return KingActions;
	}
	bool InActionPath( const int actionNum, const int actionX, const int actionY ) const override;
};

class Queen : public Piece {
public:
	Queen( const teamCode_t team ) : Piece() {
		this->type = pieceType_t::QUEEN;
		this->team = team;
		this->numActions = static_cast<int>( moveType_t::QUEEN_ACTIONS );
	}
	const moveAction_t* GetActions() const {
		return QueenActions;
	}
};