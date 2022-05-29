#include <genesis.h>
#include "board.h"
#include "resources.h"
#include "swirls.h"

static void twist_init_board( u16* board ) {
	for( int y = 0; y < TWIST_SWIRLS_Y; y++ ) {
		for( int x = 0; x < TWIST_SWIRLS_X; x++ ) {
			board[ TWIST_SWIRLS_X * y + x ] = random() % 4;
		}
	}
}

static void twist_draw_board( u16* board ) {
	VDP_clearPlane( BG_A, FALSE );
	VDP_clearPlane( BG_B, FALSE );

	VDP_drawText( "Mega", 33, 1 );
	VDP_drawText( "Twist", 32, 2 );

	VDP_drawText( "Score:", 31, 26 );
	VDP_drawText( "0", 31, 27 );

	for( int y = 0; y < TWIST_SWIRLS_Y; y++ ) {
		for( int x = 0; x < TWIST_SWIRLS_X; x++ ) {
			VDP_fillTileMapRectInc(
				BG_B,
				TILE_ATTR_FULL( PAL1, FALSE, FALSE, FALSE, TILE_USER_INDEX + ( board[ TWIST_SWIRLS_X * y + x ] * 4 ) ),
				x * 2,
				y * 2,
				2,
				2
			);
		}
	}
}

static void twist_load_swirl_gfx() {
	VDP_loadTileSet( twist_swirl1.tileset, TILE_USER_INDEX, CPU );
	VDP_loadTileSet( twist_swirl2.tileset, TILE_USER_INDEX + 4, CPU );
	VDP_loadTileSet( twist_swirl3.tileset, TILE_USER_INDEX + 8, CPU );
	VDP_loadTileSet( twist_swirl4.tileset, TILE_USER_INDEX + 12, CPU );
}

void twist_gameplay() {
	u16 board[ TWIST_SWIRLS_X * TWIST_SWIRLS_Y ] = { 0 };

	twist_load_swirl_gfx();
	twist_init_board( board );
	twist_draw_board( board );

	while( TRUE ) {
		SYS_doVBlankProcess();
	}
}