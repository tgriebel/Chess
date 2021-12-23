#pragma once
#include "common.h"
#include "chessState.h"
#include "piece.h"

class Chess {
public:
	Chess( const gameConfig_t& cfg ) {
		pieceNum = 0;
		winner = teamCode_t::NONE;
		inCheck = teamCode_t::NONE;
		memset( s.pieces, 0, sizeof( Piece* ) * PieceCount );
		config = cfg;
		SetBoard( config );
		s.game = this;
		s.CountTeamPieces();
	}

	~Chess() {
		pieceNum = 0;
		for ( int i = 0; i < PieceCount; ++i ) {
			delete s.pieces[ i ];
		}
	}

	static Piece* CreatePiece( const pieceType_t pieceType, const teamCode_t teamCode );

	static inline teamCode_t GetOpposingTeam( const teamCode_t team ) {
		return ( team == teamCode_t::WHITE ) ? teamCode_t::BLACK : teamCode_t::WHITE;
	}

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

	void GetTeamCaptures( const teamCode_t teamCode, pieceInfo_t capturedPieces[ TeamPieceCount ], int& captureCount ) const {
		memset( capturedPieces, 0, sizeof( capturedPieces[ 0 ] ) * TeamPieceCount );
		const int index = static_cast<int>( teamCode );
		if ( ( index >= 0 ) && ( index < TeamCount ) ) {
			captureCount = s.teams[ index ].capturedCount;
			for ( int i = 0; i < s.teams[ index ].capturedCount; ++i ) {
				capturedPieces[ i ] = GetInfo( s.teams[ index ].captured[ i ] );
			}
		}
	}

	void EnumerateActions( const pieceHandle_t pieceHdl, std::vector< moveAction_t >& actionList ) const {
		const Piece* piece = s.GetPiece( pieceHdl );
		if ( piece != nullptr ) {
			const int actionCount = piece->GetActionCount();
			for ( int action = 0; action < actionCount; ++action ) {
				moveAction_t path[ BoardSize ];
				const int pathLength = piece->GetActionPath( action, path );
				for ( int i = 0; i < pathLength; ++i ) {
					actionList.push_back( path[ i ] );
				}
			}
		}
	}

	inline const pieceHandle_t FindPiece( const teamCode_t team, const pieceType_t type, const int instance ) const {
		return const_cast<Chess*>( this )->FindPiece( team, type, instance );
	}

	pieceHandle_t		FindPiece( const teamCode_t team, const pieceType_t type, const int instance );
	pieceInfo_t			GetInfo( const pieceHandle_t pieceType ) const;
	pieceInfo_t			GetInfo( const int x, const int y ) const;
	void				SetEventCallback( callback_t callback ) { this->s.callback = callback; }
	inline teamCode_t	GetWinner() const { return winner; }
	inline int			GetPieceCount() const { return pieceNum; }
	bool				IsValidHandle( const pieceHandle_t handle ) const;

private:
	void				SetBoard( const gameConfig_t& cfg );
	void				EnterPieceInGame( Piece* piece, const int x, const int y );
	bool				PerformMoveAction( const pieceHandle_t pieceHdl, const int targetX, const int targetY );
private:
	ChessState			s;
	int					pieceNum;
	teamCode_t			winner;
	teamCode_t			inCheck;
	gameConfig_t		config;
};