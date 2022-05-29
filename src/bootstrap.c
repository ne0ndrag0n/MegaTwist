#include <genesis.h>
#include "bootstrap.h"
#include "resources.h"
#include "swirls.h"
#include "titlescreen.h"
#include "board.h"


void twist_bootstrap( u16 resetType ) {
	if( !resetType ) {
		SYS_hardReset();
	}

	VDP_init();

	JOY_init();

	SPR_init();
	SPR_clear();

	VDP_setScreenWidth320();

	VDP_loadFontData( twist_font_noted.tileset->tiles, 96, CPU );

	PAL_setColors( 0, twist_font_noted.palette->data, 16, CPU );
	PAL_setColors( 16, twist_swirl1.palette->data, 16, CPU );

	while( TRUE ) {
		twist_titlescreen();
		twist_gameplay();
	}
}