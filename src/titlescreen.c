#include <genesis.h>
#include "resources.h"
#include "titlescreen.h"
#include "swirls.h"

static u8 waitFlag = TRUE;

static void twist_titlescreen_handler( u16 joy, u16 changed, u16 state ) {
	switch (joy ) {
		case JOY_1: {
			if ( state & BUTTON_START ) {
				waitFlag = FALSE;
			}
		}
	}
}

void twist_titlescreen() {
	JOY_setEventHandler( twist_titlescreen_handler );

	VDP_drawText( "0.0.1a - built "__DATE__" "__TIME__, 0, 27 );
	VDP_drawText( "Lady Empress Zoe's", 10, 10 );
	VDP_drawText( "Mega Twist", 15, 11 );
	VDP_drawText( "Press Start", 14, 13 );

	u16** frames[ 4 ];
	frames[ 0 ] = SPR_loadAllFrames( &twist_swirl1_sprite, TILE_USER_INDEX, NULL );
	frames[ 1 ] = SPR_loadAllFrames( &twist_swirl2_sprite, TILE_USER_INDEX + 4, NULL );
	frames[ 2 ] = SPR_loadAllFrames( &twist_swirl3_sprite, TILE_USER_INDEX + 8, NULL );
	frames[ 3 ] = SPR_loadAllFrames( &twist_swirl4_sprite, TILE_USER_INDEX + 12, NULL );

	Sprite* swirls[ TWIST_NUM_TS_SWIRLS ];
	for( int i = 0; i < TWIST_NUM_TS_SWIRLS; i += 4 ) {
		swirls[ i ]     = SPR_addSpriteExSafe( &twist_swirl1_sprite, random() % 320, ( random() % TWIST_START_Y_TS ) * -1, TILE_ATTR( PAL1, FALSE, FALSE, FALSE ), NULL, SPR_FLAG_AUTO_VISIBILITY | SPR_FLAG_AUTO_SPRITE_ALLOC | SPR_FLAG_AUTO_TILE_UPLOAD );
		swirls[ i + 1 ] = SPR_addSpriteExSafe( &twist_swirl2_sprite, random() % 320, ( random() % TWIST_START_Y_TS ) * -1, TILE_ATTR( PAL1, FALSE, FALSE, FALSE ), NULL, SPR_FLAG_AUTO_VISIBILITY | SPR_FLAG_AUTO_SPRITE_ALLOC | SPR_FLAG_AUTO_TILE_UPLOAD );
		swirls[ i + 2 ] = SPR_addSpriteExSafe( &twist_swirl3_sprite, random() % 320, ( random() % TWIST_START_Y_TS ) * -1, TILE_ATTR( PAL1, FALSE, FALSE, FALSE ), NULL, SPR_FLAG_AUTO_VISIBILITY | SPR_FLAG_AUTO_SPRITE_ALLOC | SPR_FLAG_AUTO_TILE_UPLOAD );
		swirls[ i + 3 ] = SPR_addSpriteExSafe( &twist_swirl4_sprite, random() % 320, ( random() % TWIST_START_Y_TS ) * -1, TILE_ATTR( PAL1, FALSE, FALSE, FALSE ), NULL, SPR_FLAG_AUTO_VISIBILITY | SPR_FLAG_AUTO_SPRITE_ALLOC | SPR_FLAG_AUTO_TILE_UPLOAD );

		SPR_setVRAMTileIndex( swirls[ i ],     TILE_USER_INDEX );
		SPR_setVRAMTileIndex( swirls[ i + 1 ], TILE_USER_INDEX + 4 );
		SPR_setVRAMTileIndex( swirls[ i + 2 ], TILE_USER_INDEX + 8 );
		SPR_setVRAMTileIndex( swirls[ i + 3 ], TILE_USER_INDEX + 12 );
	}

	while( waitFlag ) {
		for( int i = 0; i < TWIST_NUM_TS_SWIRLS; i++ ) {
			s16 x = swirls[ i ]->x - 0x80;
			s16 y = swirls[ i ]->y - 0x80;

			if( y > 224 ) {
				x = random() % 320;
				y = ( random() % TWIST_START_Y_TS ) * -1;
			} else {
				y += 2;
			}

			SPR_setPosition( swirls[ i ], x, y );
		}

		SPR_update();
		SYS_doVBlankProcess();
	}

	// All those calls to SPR_loadAllFrames return unused tables which must be freed
	SPR_reset();
	SPR_update();
	for( int i = 0; i < 4; i++ ) {
		MEM_free( frames[ i ] );
	}

	VDP_clearPlane( BG_A, FALSE );

	// Exit titlescreen to go to gameplay
}