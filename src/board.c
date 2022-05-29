#include <genesis.h>
#include <maths.h>
#include "board.h"
#include "resources.h"
#include "swirls.h"

static void twist_init_board( Swirl* board ) {
	for( int y = 0; y < TWIST_SWIRLS_Y; y++ ) {
		for( int x = 0; x < TWIST_SWIRLS_X; x++ ) {
			board[ TWIST_SWIRLS_X * y + x ] = ( Swirl ) { .type = random() % 4, .selected = FALSE };
		}
	}
}

static void twist_draw_board( Swirl* board, s32 score ) {
	VDP_clearPlane( BG_A, FALSE );
	VDP_clearPlane( BG_B, FALSE );

	VDP_drawText( "Twist", 33, 1 );

	VDP_drawText( "Score:", 31, 26 );

	char scoreString[ 11 ] = { 0 };
	sprintf( scoreString, "%d", score );
	VDP_drawText( scoreString, 31, 27 );

	for( int y = 0; y < TWIST_SWIRLS_Y; y++ ) {
		for( int x = 0; x < TWIST_SWIRLS_X; x++ ) {
			VDP_fillTileMapRectInc(
				BG_B,
				TILE_ATTR_FULL( PAL1, FALSE, FALSE, FALSE, TILE_USER_INDEX + ( board[ TWIST_SWIRLS_X * y + x ].type * 4 ) ),
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

static u16 joyDump = 0;

static void twist_board_handler( u16 joy, u16 changed, u16 state ) {
	switch (joy ) {
		case JOY_1: {
			joyDump = state;
		}
	}
}

static void twist_update_animations( Sprite* onScreenCursor ) {
	static Vect2D_s16 animDirection = ( Vect2D_s16 ){ .x = 0, .y = 0 };
	static u16 remaining = 0;

	if( remaining > 0 ) {
		SPR_setPosition( onScreenCursor, onScreenCursor->x - 0x80 + ( animDirection.x * 4 ), onScreenCursor->y - 0x80 + ( animDirection.y * 4 ) );

		remaining--;
	} else {
		// Check joyDump for the need to play a new animation
		Vect2D_s16 currentPosition = ( Vect2D_s16 ){ .x = onScreenCursor->x - 0x80, .y = onScreenCursor->y - 0x80 };
		if( joyDump & BUTTON_RIGHT && currentPosition.x < 224 ) {
			animDirection = ( Vect2D_s16 ){ .x = 1, .y = 0 };
			remaining = 4;
		} else if( joyDump & BUTTON_LEFT && currentPosition.x > 0 ) {
			animDirection = ( Vect2D_s16 ){ .x = -1, .y = 0 };
			remaining = 4;
		} else if( joyDump & BUTTON_UP && currentPosition.y > 0 ) {
			animDirection = ( Vect2D_s16 ){ .x = 0, .y = -1 };
			remaining = 4;
		} else if( joyDump & BUTTON_DOWN && currentPosition.y < 208 ) {
			animDirection = ( Vect2D_s16 ){ .x = 0, .y = 1 };
			remaining = 4;
		}
	}
}

void twist_gameplay() {
	JOY_setEventHandler( twist_board_handler );

	Swirl board[ TWIST_SWIRLS_X * TWIST_SWIRLS_Y ] = { 0 };
	s32 score = 0;

	twist_load_swirl_gfx();
	twist_init_board( board );
	twist_draw_board( board, score );

	u16** frames = SPR_loadAllFrames( &cursor, TILE_USER_INDEX + 16, NULL );
	Sprite* onScreenCursor = SPR_addSpriteExSafe(
		&cursor,
		0,
		0,
		TILE_ATTR( PAL1, TRUE, FALSE, FALSE ),
		NULL,
		SPR_FLAG_AUTO_VISIBILITY | SPR_FLAG_AUTO_SPRITE_ALLOC | SPR_FLAG_AUTO_TILE_UPLOAD
	);
	SPR_setVRAMTileIndex( onScreenCursor, TILE_USER_INDEX + 16 );

	while( TRUE ) {
		twist_update_animations( onScreenCursor );

		SPR_update();
		SYS_doVBlankProcess();
	}

	MEM_free( frames );
}