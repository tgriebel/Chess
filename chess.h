#pragma once
#include <assert.h>

static const int BoardSize = 8;
typedef int pieceHandle_t;
static const pieceHandle_t NoPiece = -1;
static const pieceHandle_t OffBoard = -2;

enum class pieceType_t : int {
	NONE = -1,
	PAWN = 0,
	ROOK,
	KNIGHT,
	BISHOP,
	KING,
	QUEEN,
	COUNT,
};

enum class teamCode_t : int {
	NONE = -1,
	WHITE = 0,
	BLACK = 1,
	COUNT
};

enum moveType_t : int {
	NONE = -1,

	PAWN_T = 0,
	PAWN_T2X,
	PAWN_KILL_L,
	PAWN_KILL_R,
	PAWN_ACTIONS,

	KNIGHT_T1L2 = 0,
	KNIGHT_T2L1,
	KNIGHT_T1R2,
	KNIGHT_T2R1,
	KNIGHT_B1R2,
	KNIGHT_B2R1,
	KNIGHT_B2L1,
	KNIGHT_B1L2,
	KNIGHT_ACTIONS,

	ROOK_L = 0,
	ROOK_R,
	ROOK_T,
	ROOK_B,
	ROOK_ACTIONS,

	BISHOP_TL = 0,
	BISHOP_TR,
	BISHOP_BR,
	BISHOP_BL,
	BISHOP_ACTIONS,

	KING_TL = 0,
	KING_T,
	KING_TR,
	KING_R,
	KING_BR,
	KING_B,
	KING_BL,
	KING_L,
	KING_CASTLE_L,
	KING_CASTLE_R,
	KING_ACTIONS,

	QUEEN_TL = 0,
	QUEEN_T,
	QUEEN_TR,
	QUEEN_R,
	QUEEN_BR,
	QUEEN_B,
	QUEEN_BL,
	QUEEN_L,
	QUEEN_ACTIONS,

	MOVE_COUNT = ( PAWN_ACTIONS + KNIGHT_ACTIONS + ROOK_ACTIONS + BISHOP_ACTIONS + KING_ACTIONS + QUEEN_ACTIONS ),
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
	team_t() {
		for ( int i = 0; i < 16; ++i ) {
			pieces[ i ] = 0;
			captured[ i ] = 0;
		}
		for ( int i = 0; i < (int)pieceType_t::COUNT; ++i ) {
			typeCounts[ i ] = 0;
			captureTypeCounts[ i ] = 0;
		}
		livingCount = 0;
		capturedCount = 0;
	}

	pieceHandle_t	pieces[ 16 ];
	pieceHandle_t	captured[ 16 ];
	int				livingCount;
	int				capturedCount;
	int				typeCounts[ (int)pieceType_t::COUNT ];
	int				captureTypeCounts[ (int)pieceType_t::COUNT ];
};

struct squareCfg_t {
	teamCode_t	team;
	pieceType_t	piece;
};

#define BP { teamCode_t::BLACK, pieceType_t::PAWN }
#define BR { teamCode_t::BLACK, pieceType_t::ROOK }
#define BN { teamCode_t::BLACK, pieceType_t::KNIGHT }
#define BB { teamCode_t::BLACK, pieceType_t::BISHOP }
#define BQ { teamCode_t::BLACK, pieceType_t::QUEEN }
#define BK { teamCode_t::BLACK, pieceType_t::KING }
#define WP { teamCode_t::WHITE, pieceType_t::PAWN }
#define WR { teamCode_t::WHITE, pieceType_t::ROOK }
#define WN { teamCode_t::WHITE, pieceType_t::KNIGHT }
#define WB { teamCode_t::WHITE, pieceType_t::BISHOP }
#define WQ { teamCode_t::WHITE, pieceType_t::QUEEN }
#define WK { teamCode_t::WHITE, pieceType_t::KING }
#define CL { teamCode_t::NONE, pieceType_t::NONE }

struct gameConfig_t {
	const squareCfg_t DefaultCfg[ BoardSize ][ BoardSize ] = {
		{ BR, BN, BB, BQ, BK, BB, BN, BR },
		{ BP, BP, BP, BP, BP, BP, BP, BP },
		{ CL, CL, CL, CL, CL, CL, CL, CL },
		{ CL, CL, CL, CL, CL, CL, CL, CL },
		{ CL, CL, CL, CL, CL, CL, CL, CL },
		{ CL, CL, CL, CL, CL, CL, CL, CL },
		{ WP, WP, WP, WP, WP, WP, WP, WP },
		{ WR, WN, WB, WQ, WK, WB, WN, WR },
	};

	gameConfig_t() {
		for ( int i = 0; i < BoardSize; ++i ) {
			for ( int j = 0; j < BoardSize; ++j ) {
				board[ i ][ j ] = DefaultCfg[ i ][ j ];
			}
		}
	}

	gameConfig_t( const gameConfig_t& config ) {
		for ( int i = 0; i < BoardSize; ++i ) {
			for ( int j = 0; j < BoardSize; ++j ) {
				board[ i ][ j ] = config.board[ i ][ j ];
			}
		}
	}

	gameConfig_t& operator=( const gameConfig_t& config ) {
		if ( this == &config ) {
			return *this;
		}

		for ( int i = 0; i < BoardSize; ++i ) {
			for ( int j = 0; j < BoardSize; ++j ) {
				board[ i ][ j ] = config.board[ i ][ j ];
			}
		}
		
		return *this;
	}

	squareCfg_t board[ BoardSize ][ BoardSize ];
};