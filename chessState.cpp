#include "game.h"
#include "chessState.h"

void ChessState::SetBoard( const gameConfig_t& cfg ) {
	for ( int i = 0; i < BoardSize; ++i ) {
		for ( int j = 0; j < BoardSize; ++j ) {
			grid[ i ][ j ] = NoPiece;
			const pieceType_t pieceType = cfg.board[ i ][ j ].piece;
			const teamCode_t teamCode = cfg.board[ i ][ j ].team;
			Piece* piece = Chess::CreatePiece( pieceType, teamCode );
			if ( piece != nullptr ) {
				EnterPieceInGame( piece, j, i );
			}
		}
	}
}