#pragma once
#include "chess.h"
#include "piece.h"

class ChessBoard {
public:
	static const int TeamCount = 2;
	static const int TeamPieceCount = 16;
	static const int PieceCount = 32;

	ChessBoard() {
		nextHandle = 0;
		// Create pieces/teams
		{
			for ( int i = 0; i < TeamCount; ++i ) {
				const teamCode_t teamCode = static_cast<teamCode_t>( i );
				for ( int j = 0; j < 8; ++j ) {
					pieces[ nextHandle++ ] = new Pawn( teamCode );
				}
				pieces[ nextHandle++ ] = new Rook( teamCode );
				pieces[ nextHandle++ ] = new Knight( teamCode );
				pieces[ nextHandle++ ] = new Bishop( teamCode );
				pieces[ nextHandle++ ] = new Queen( teamCode );
				pieces[ nextHandle++ ] = new King( teamCode );
				pieces[ nextHandle++ ] = new Bishop( teamCode );
				pieces[ nextHandle++ ] = new Knight( teamCode );
				pieces[ nextHandle++ ] = new Rook( teamCode );

				teams[ i ].livingCount = TeamPieceCount;
				teams[ i ].capturedCount = 0;
				for ( int j = 0; j < TeamPieceCount; ++j ) {
					const pieceHandle_t handle = ( i * TeamPieceCount + j );
					pieces[ handle ]->BindBoard( this, j );
					teams[ i ].pieces[ j ] = handle;
					teams[ i ].captured[ j ] = NoPiece;
				}
			}
		}

		// Create board
		{
			const int whiteIndex = static_cast<int>( teamCode_t::WHITE );
			const int blackIndex = static_cast<int>( teamCode_t::BLACK );
			for ( int i = 0; i < BoardSize; ++i ) {
				grid[ 0 ][ i ] = teams[ blackIndex ].pieces[ i + BoardSize ];
				grid[ 1 ][ i ] = teams[ blackIndex ].pieces[ i ];

				grid[ BoardSize - 2 ][ i ] = teams[ whiteIndex ].pieces[ i ];
				grid[ BoardSize - 1 ][ i ] = teams[ whiteIndex ].pieces[ i + BoardSize ];
			}

			for ( int i = 2; i < ( BoardSize - 2 ); ++i ) {
				for ( int j = 0; j < BoardSize; ++j ) {
					grid[ i ][ j ] = NoPiece;
				}
			}
		}

		// Assign starting locations
		{
			for ( int i = 0; i < BoardSize; ++i ) {
				for ( int j = 0; j < BoardSize; ++j ) {
					Piece* piece = GetPiece( grid[ i ][ j ] );
					if ( piece == nullptr ) {
						continue;
					}
					piece->x = j;
					piece->y = i;
				}
			}
		}
		CountTeamPieces();
	}

	~ChessBoard() {
		nextHandle = 0;
		for ( int i = 0; i < BoardSize; ++i ) {
			for ( int j = 0; j < BoardSize; ++j ) {
				grid[ i ][ j ] = NoPiece;
			}
		}
		for ( int i = 0; i < PieceCount; ++i ) {
			delete pieces[ i ];
		}
	}

	bool Execute( const command_t& cmd ) {
		const pieceHandle_t piece = FindPiece( cmd.team, cmd.pieceType, cmd.instance );
		if ( piece == NoPiece ) {
			return false;
		}
		return MovePiece( piece, cmd.x, cmd.y );
	}

	bool IsLegalMove( const Piece* piece, const int targetX, const int targetY ) const;

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
		return const_cast< ChessBoard* >( this )->GetPiece( handle );
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

	bool IsOpenToAttackAt( const pieceHandle_t pieceHdl, const int x, const int y ) const;

private:
	bool IsValidHandle( const pieceHandle_t handle ) const;
	pieceHandle_t GetHandle( const int x, const int y ) const;
	void CapturePiece( const teamCode_t attacker, const int x, const int y );
	bool CanPromotePawn( const Pawn* pawn ) const;
	void PromotePawn( const pieceHandle_t pieceHdl );
	bool ForcedCheckMate( const teamCode_t team ) const;
	bool MovePiece( const pieceHandle_t pieceHdl, const int targetX, const int targetY );
	void GetPieceLocation( const pieceHandle_t handle, int& x, int& y ) const;
	void CountTeamPieces();
private:
	Piece*			pieces[ PieceCount ];
	team_t			teams[ TeamCount ];
	pieceHandle_t	grid[ BoardSize ][ BoardSize ]; // (0,0) is top left
	pieceHandle_t	nextHandle;
};