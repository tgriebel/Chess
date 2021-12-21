#pragma once
#include "chess.h"
#include "piece.h"

class ChessBoard {
public:
	static const int TeamCount = 2;
	static const int TeamPieceCount = 16;
	static const int PieceCount = 32;

	ChessBoard( const gameConfig_t& cfg ) {
		pieceNum = 0;
		enableOpenAttackCheck = true;
		winner = teamCode_t::NONE;
		inCheck = teamCode_t::NONE;
		memset( pieces, 0, sizeof( Piece* ) * PieceCount );
		config = cfg;
		SetBoard( config );
		CountTeamPieces();
	}

	~ChessBoard() {
		pieceNum = 0;
		for ( int i = 0; i < PieceCount; ++i ) {
			delete pieces[ i ];
		}
	}

	void SetBoard( const gameConfig_t& cfg ) {
		for ( int i = 0; i < BoardSize; ++i ) {
			for ( int j = 0; j < BoardSize; ++j ) {
				grid[ i ][ j ] = NoPiece;
				const pieceType_t piece = cfg.board[ i ][ j ].piece;
				const teamCode_t teamCode = cfg.board[ i ][ j ].team;
				switch ( piece ) {
				case pieceType_t::PAWN:
					SetPiece( new Pawn( teamCode ), j, i );
					break;
				case pieceType_t::ROOK:
					SetPiece( new Rook( teamCode ), j, i );
					break;
				case pieceType_t::KNIGHT:
					SetPiece( new Knight( teamCode ), j, i );
					break;
				case pieceType_t::BISHOP:
					SetPiece( new Bishop( teamCode ), j, i );
					break;
				case pieceType_t::QUEEN:
					SetPiece( new Queen( teamCode ), j, i );
					break;
				case pieceType_t::KING:
					SetPiece( new King( teamCode ), j, i );
					break;
				}
			}
		}
	}

	void SetPiece( Piece* piece, const int x, const int y ) {
		pieces[ pieceNum ] = piece;
		pieces[ pieceNum ]->BindBoard( this, pieceNum );
		pieces[ pieceNum ]->Set( x, y );

		const int teamIndex = static_cast<int>( piece->team );
		const int pieceIndex = teams[ teamIndex ].livingCount;
		teams[ teamIndex ].pieces[ pieceIndex ] = pieceNum;
		++teams[ teamIndex ].livingCount;

		grid[ y ][ x ] = pieceNum;
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

	moveType_t IsLegalMove( const Piece* piece, const int targetX, const int targetY ) const;

	teamCode_t GetTeam( const int x, const int y ) const {
		const Piece* targetPiece = GetPiece( x, y );
		const bool isOccupied = ( targetPiece != nullptr );
		return ( targetPiece != nullptr ) ? targetPiece->team : teamCode_t::NONE;
	}

	inline bool IsOnBoard( const int x, const int y ) const {
		return ( x >= 0 ) && ( x < BoardSize ) && ( y >= 0 ) && ( y < BoardSize );
	}

	inline const pieceHandle_t FindPiece( const teamCode_t team, const pieceType_t type, const int instance ) const {
		return const_cast<ChessBoard*>( this )->FindPiece( team, type, instance );
	}

	pieceHandle_t FindPiece( const teamCode_t team, const pieceType_t type, const int instance );

	inline const Piece* GetPiece( const pieceHandle_t handle ) const {
		return const_cast<ChessBoard*>( this )->GetPiece( handle );
	}

	inline Piece* GetPiece( const pieceHandle_t handle ) {
		if ( IsValidHandle( handle ) ) {
			return pieces[ handle ];
		}
		return nullptr;
	}

	inline const Piece* GetPiece( const int x, const int y ) const {
		return const_cast<ChessBoard*>( this )->GetPiece( x, y );
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

	inline teamCode_t GetWinner() const {
		return winner;
	}

	void MovePiece( Piece* piece, const int targetX, const int targetY );
	bool IsOpenToAttackAt( const pieceHandle_t pieceHdl, const int x, const int y ) const;

private:
	bool IsValidHandle( const pieceHandle_t handle ) const;
	pieceHandle_t GetHandle( const int x, const int y ) const;
	void CapturePiece( const teamCode_t attacker, const int x, const int y );
	bool CanPromotePawn( const Pawn* pawn ) const;
	void PromotePawn( const pieceHandle_t pieceHdl );
	bool ForcedCheckMate( const teamCode_t team ) const;
	bool PerformMoveAction( const pieceHandle_t pieceHdl, const int targetX, const int targetY );
	void CountTeamPieces();
private:
	teamCode_t		inCheck;
	teamCode_t		winner;
	mutable bool	enableOpenAttackCheck; // Disables recursion
	int				pieceNum;
	Piece*			pieces[ PieceCount ];
	team_t			teams[ TeamCount ];
	pieceHandle_t	grid[ BoardSize ][ BoardSize ]; // (0,0) is top left

	gameConfig_t	config;
};