#include <genesis.h>
#include "resources.h"
#include "titlescreen.h"

void twist_titlescreen() {

	VDP_drawText( "0.0.1a - built "__DATE__" "__TIME__, 0, 27 );
	VDP_drawText( "Lady Empress Zoe's", 10, 10 );
	VDP_drawText( "Mega Twist", 15, 11 );
	VDP_drawText( "Press Start", 14, 13 );

	Sprite* swirls[ 24 ];
	for( int i = 0; i < 24; i += 4 ) {
		swirls[ i ] = SPR_addSprite( &twist_swirl1_sprite, random() % 320, ( random() % 120 ) * -1, TILE_ATTR( PAL1, TRUE, FALSE, FALSE ) );
		swirls[ i + 1 ] = SPR_addSprite( &twist_swirl2_sprite, random() % 320, ( random() % 120 ) * -1, TILE_ATTR( PAL1, TRUE, FALSE, FALSE ) );
		swirls[ i + 2 ] = SPR_addSprite( &twist_swirl3_sprite, random() % 320, ( random() % 120 ) * -1, TILE_ATTR( PAL1, TRUE, FALSE, FALSE ) );
		swirls[ i + 3 ] = SPR_addSprite( &twist_swirl4_sprite, random() % 320, ( random() % 120 ) * -1, TILE_ATTR( PAL1, TRUE, FALSE, FALSE ) );
	}

	while( 1 ) {
		for( int i = 0; i < 24; i++ ) {
			s16 x = swirls[ i ]->x - 0x80;
			s16 y = swirls[ i ]->y - 0x80;

			if( y > 224 ) {
				x = random() % 320;
				y = ( random() % 120 ) * -1;
			} else {
				y += 2;
			}

			SPR_setPosition( swirls[ i ], x, y );
		}

		SPR_update();
		SYS_doVBlankProcess();
	}
}