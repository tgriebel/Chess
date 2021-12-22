#pragma once
#include "chess.h"
#include "piece.h"

typedef void ( *callback_t )( callbackEvent_t& );

class Chess {
public:
	static const int TeamCount = 2;
	static const int TeamPieceCount = 16;
	static const int PieceCount = 32;

	Chess( const gameConfig_t& cfg ) {
		pieceNum = 0;
		winner = teamCode_t::NONE;
		inCheck = teamCode_t::NONE;
		memset( pieces, 0, sizeof( Piece* ) * PieceCount );
		config = cfg;
		SetBoard( config );
		CountTeamPieces();
	}

	~Chess() {
		pieceNum = 0;
		for ( int i = 0; i < PieceCount; ++i ) {
			delete pieces[ i ];
		}
	}

	void SetBoard( const gameConfig_t& cfg ) {
		for ( int i = 0; i < BoardSize; ++i ) {
			for ( int j = 0; j < BoardSize; ++j ) {
				grid[ i ][ j ] = NoPiece;
				const pieceType_t pieceType = cfg.board[ i ][ j ].piece;
				const teamCode_t teamCode = cfg.board[ i ][ j ].team;
				Piece* piece = CreatePiece( pieceType, teamCode );
				if ( piece != nullptr ) {
					SetPiece( piece, j, i );
				}
			}
		}
	}

	Piece* CreatePiece( const pieceType_t pieceType, const teamCode_t teamCode ) {
		switch ( pieceType ) {
		case pieceType_t::PAWN:		return new Pawn( teamCode );
		case pieceType_t::ROOK:		return new Rook( teamCode );
		case pieceType_t::KNIGHT:	return new Knight( teamCode );
		case pieceType_t::BISHOP:	return new Bishop( teamCode );
		case pieceType_t::QUEEN:	return new Queen( teamCode );
		case pieceType_t::KING:		return new King( teamCode );
		}
		return nullptr;
	}

	void SetPiece( Piece* piece, const int x, const int y ) {
		pieces[ pieceNum ] = piece;
		pieces[ pieceNum ]->BindBoard( this, pieceNum );
		pieces[ pieceNum ]->Set( x, y );

		const int teamIndex = static_cast<int>( piece->team );
		const int pieceIndex = teams[ teamIndex ].livingCount;
		teams[ teamIndex ].pieces[ pieceIndex ] = pieceNum;
		++teams[ teamIndex ].livingCount;
		++pieceNum;
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

	inline teamCode_t GetOpposingTeam( const teamCode_t team ) const {
		return ( team == teamCode_t::WHITE ) ? teamCode_t::BLACK : teamCode_t::WHITE;
	}

	inline const pieceHandle_t FindPiece( const teamCode_t team, const pieceType_t type, const int instance ) const {
		return const_cast<Chess*>( this )->FindPiece( team, type, instance );
	}

	pieceHandle_t FindPiece( const teamCode_t team, const pieceType_t type, const int instance );

	inline const Piece* GetPiece( const pieceHandle_t handle ) const {
		return const_cast<Chess*>( this )->GetPiece( handle );
	}

	inline Piece* GetPiece( const pieceHandle_t handle ) {
		if ( IsValidHandle( handle ) ) {
			return pieces[ handle ];
		}
		return nullptr;
	}

	inline const Piece* GetPiece( const int x, const int y ) const {
		return const_cast<Chess*>( this )->GetPiece( x, y );
	}

	inline Piece* GetPiece( const int x, const int y ) {
		const pieceHandle_t handle = GetHandle( x, y );
		if ( IsValidHandle( handle ) == false ) {
			return nullptr;
		}
		return pieces[ handle ];
	}

	inline void GetTeamCaptures( const teamCode_t teamCode, const Piece* capturedPieces[ TeamPieceCount ], int& captureCount ) const {
		const int index = static_cast<int>( teamCode );
		if ( ( index >= 0 ) && ( index < TeamCount ) ) {
			captureCount = teams[ index ].capturedCount;
			for ( int i = 0; i < teams[ index ].capturedCount; ++i ) {
				capturedPieces[ i ] = GetPiece( teams[ index ].captured[ i ] );
			}
		}
	}

	void SetEventCallback( callback_t callback ) {
		this->callback = callback;
	}

	inline teamCode_t GetWinner() const {
		return winner;
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
	bool PerformMoveAction( const pieceHandle_t pieceHdl, const int targetX, const int targetY );
	void CountTeamPieces();
private:
	teamCode_t		inCheck;
	teamCode_t		winner;
	pieceHandle_t	enpassantPawn;
	int				pieceNum;
	Piece*			pieces[ PieceCount ];
	team_t			teams[ TeamCount ];
	pieceHandle_t	grid[ BoardSize ][ BoardSize ]; // (0,0) is top left
	callback_t		callback;

	gameConfig_t	config;

	friend class Piece;
	friend class Pawn;
};