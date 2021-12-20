#pragma once

static const int BoardSize = 8;
typedef int pieceHandle_t;
static const pieceHandle_t NoPiece = -1;
static const pieceHandle_t OffBoard = -2;

enum class pieceType_t : int {
	NONE = -1,
	PAWN = 0,
	ROOK = 1,
	KNIGHT = 2,
	BISHOP = 3,
	KING = 4,
	QUEEN = 5,
};

enum class teamCode_t : int {
	UNASSIGNED = -1,
	WHITE = 0,
	BLACK = 1,
	COUNT
};

enum moveType_t : int {
	PAWN_T,
	PAWN_T2X,
	PAWN_KILL_L,
	PAWN_KILL_R,

	KNIGHT_T1L2,
	KNIGHT_T2L1,
	KNIGHT_T1R2,
	KNIGHT_T2R1,
	KNIGHT_B1R2,
	KNIGHT_B2R1,
	KNIGHT_B2L1,
	KNIGHT_B1L2,

	ROOK_L,
	ROOK_R,
	ROOK_T,
	ROOK_B,

	BISHOP_TL,
	BISHOP_TR,
	BISHOP_BR,
	BISHOP_BL,

	KING_TL,
	KING_T,
	KING_TR,
	KING_R,
	KING_BR,
	KING_B,
	KING_BL,
	KING_L,

	QUEEN_TL,
	QUEEN_T,
	QUEEN_TR,
	QUEEN_R,
	QUEEN_BR,
	QUEEN_B,
	QUEEN_BL,
	QUEEN_L,

	MOVE_COUNT,
	NONE,
};

struct moveAction_t {
	moveAction_t() {
		x = 0;
		y = 0;
		type = moveType_t::NONE;
	}
	moveAction_t( const int x, int const y, const moveType_t type ) :
		x( x ), y( y ), type( type ) {}
	int				x;
	int				y;
	moveType_t	type;
};

struct command_t {
	int				x;
	int				y;
	pieceType_t		pieceType;
	int				instance;
	teamCode_t		team;
};

struct team_t {
	pieceHandle_t	pieces[ 16 ];
	pieceHandle_t	captured[ 16 ];
	int				livingCount;
	int				capturedCount;
};