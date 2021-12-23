#pragma once
#include "chess.h"
#include "chessState.h"
#include "piece.h"

class Chess {
public:
	Chess( const gameConfig_t& cfg ) {
		s = new ChessState();
		s->pieceNum = 0;
		winner = teamCode_t::NONE;
		inCheck = teamCode_t::NONE;
		memset( s->pieces, 0, sizeof( Piece* ) * PieceCount );
		config = cfg;
		SetBoard( config );
		s->game = this;
		s->CountTeamPieces();
	}

	~Chess() {
		s->pieceNum = 0;
		for ( int i = 0; i < PieceCount; ++i ) {
			delete s->pieces[ i ];
		}
		delete s;
	}

	void SetBoard( const gameConfig_t& cfg );

	static Piece* CreatePiece( const pieceType_t pieceType, const teamCode_t teamCode );

	resultCode_t Execute( const command_t& cmd ) {
		const pieceHandle_t piece = FindPiece( cmd.team, cmd.pieceType, cmd.instance );
		if ( piece == NoPiece ) {
			return RESULT_GAME_INVALID_PIECE;
		}
		if ( GetWinner() != teamCode_t::NONE ) {
			return RESULT_GAME_COMPLETE;
		}
		if ( PerformMoveAction( piece, cmd.x, cmd.y ) == false ) {
			return RESULT_GAME_INVALID_MOVE;
		}
		return ( GetWinner() != teamCode_t::NONE ) ? RESULT_GAME_COMPLETE : RESULT_SUCCESS;
	}

	inline static teamCode_t GetOpposingTeam( const teamCode_t team ) {
		return ( team == teamCode_t::WHITE ) ? teamCode_t::BLACK : teamCode_t::WHITE;
	}

	inline const pieceHandle_t FindPiece( const teamCode_t team, const pieceType_t type, const int instance ) const {
		return const_cast<Chess*>( this )->FindPiece( team, type, instance );
	}

	pieceHandle_t FindPiece( const teamCode_t team, const pieceType_t type, const int instance );

	void GetTeamCaptures( const teamCode_t teamCode, const Piece* capturedPieces[ TeamPieceCount ], int& captureCount ) const {
		const int index = static_cast<int>( teamCode );
		if ( ( index >= 0 ) && ( index < TeamCount ) ) {
			captureCount = s->teams[ index ].capturedCount;
			for ( int i = 0; i < s->teams[ index ].capturedCount; ++i ) {
				capturedPieces[ i ] = s->GetPiece( s->teams[ index ].captured[ i ] );
			}
		}
	}

	void SetEventCallback( callback_t callback ) {
		this->s->callback = callback;
	}

	inline teamCode_t GetWinner() const {
		return winner;
	}

//	bool IsValidHandle( const pieceHandle_t handle ) const;

private:
	bool PerformMoveAction( const pieceHandle_t pieceHdl, const int targetX, const int targetY );
	//private:
public:

	ChessState* s;
	teamCode_t		winner;
	teamCode_t		inCheck;
	gameConfig_t	config;

	friend class Piece;
	friend class Pawn;
};