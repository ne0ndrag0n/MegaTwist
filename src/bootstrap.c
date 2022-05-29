#include <genesis.h>
#include "bootstrap.h"
#include "resources.h"
#include "titlescreen.h"

void twist_bootstrap() {
	VDP_setScreenWidth320();
	SPR_init();

	VDP_loadFontData( twist_font_noted.tileset->tiles, 96, CPU );

	PAL_setColors( 0, twist_font_noted.palette->data, 16, CPU );
	PAL_setColors( 16, twist_swirl1.palette->data, 16, CPU );

	twist_titlescreen();
}