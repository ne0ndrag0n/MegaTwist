#include <genesis.h>
#include <maths.h>
#include "board.h"
#include "resources.h"
#include "swirls.h"

static void twist_init_board( Board* game_state ) {
	memcpy( game_state->select_anim_palette, twist_swirl1.palette->data, 32 );
	SPR_setVRAMTileIndex( game_state->on_screen_cursor, TILE_USER_INDEX + 16 );

	for( int y = 0; y < TWIST_SWIRLS_Y; y++ ) {
		for( int x = 0; x < TWIST_SWIRLS_X; x++ ) {
			game_state->board[ TWIST_SWIRLS_X * y + x ] = ( Swirl ) {
				.type = random() % 4,
				.selected = FALSE,
				( PaletteAnimation ) {
					.type = NO_ANIMATION,
					.remaining_steps = 0
				}
			};
		}
	}
}

static void twist_draw_board( Board* game_state ) {
	if( game_state->dirty == TRUE ) {
		game_state->dirty = FALSE;

		VDP_drawText( "Twist", 33, 1 );

		VDP_drawText( "Score:", 31, 26 );

		char scoreString[ 11 ] = { 0 };
		sprintf( scoreString, "%d", game_state->score );
		VDP_drawText( scoreString, 31, 27 );

		for( int y = 0; y < TWIST_SWIRLS_Y; y++ ) {
			for( int x = 0; x < TWIST_SWIRLS_X; x++ ) {
				int index = TWIST_SWIRLS_X * y + x;

				if( game_state->board[ index ].type != NO_SWIRL ) {
					VDP_fillTileMapRectInc(
						BG_B,
						TILE_ATTR_FULL(
							PAL1,
							FALSE,
							FALSE,
							FALSE,
							TILE_USER_INDEX + ( game_state->board[ index ].type * 4 )
						),
						x * 2,
						y * 2,
						2,
						2
					);
				} else {
					VDP_fillTileMapRect(
						BG_B,
						TILE_ATTR_FULL(
							game_state->board[ index ].palette_animation.type == NO_ANIMATION ? PAL1 : PAL2,
							FALSE,
							FALSE,
							FALSE,
							0
						),
						x * 2,
						y * 2,
						2,
						2
					);
				}
			}
		}
	}
}

static void twist_load_swirl_gfx() {
	VDP_loadTileSet( twist_swirl1.tileset, TILE_USER_INDEX, CPU );
	VDP_loadTileSet( twist_swirl2.tileset, TILE_USER_INDEX + 4, CPU );
	VDP_loadTileSet( twist_swirl3.tileset, TILE_USER_INDEX + 8, CPU );
	VDP_loadTileSet( twist_swirl4.tileset, TILE_USER_INDEX + 12, CPU );

	PAL_setColors( 16, twist_swirl1.palette->data, 16, CPU );
	PAL_setColors( 32, twist_swirl1.palette->data, 16, CPU );
}

static u16 joyDump = 0;

static void twist_board_handler( u16 joy, u16 changed, u16 state ) {
	switch ( joy ) {
		case JOY_1: {
			joyDump = state;
		}
	}
}

static void twist_update_cursor( Board* game_state ) {
	static Vect2D_s16 animDirection = ( Vect2D_s16 ){ .x = 0, .y = 0 };
	static u16 remaining = 0;

	if( remaining > 0 ) {
		SPR_setPosition(
			game_state->on_screen_cursor,
			game_state->on_screen_cursor->x - 0x80 + ( animDirection.x * 4 ),
			game_state->on_screen_cursor->y - 0x80 + ( animDirection.y * 4 )
		);

		remaining--;
	} else {
		// Check joyDump for the need to play a new animation
		Vect2D_s16 currentPosition = ( Vect2D_s16 ){ .x = game_state->on_screen_cursor->x - 0x80, .y = game_state->on_screen_cursor->y - 0x80 };
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

static void twist_execute_events( Board* game_state, void* events_pointer ) {
	BoardEvent* events = ( BoardEvent* ) events_pointer;

	for( int i = 0; i < TWIST_MAX_EVENTS; i++ ) {
		if( events[ i ] ) {
			events[ i ]( game_state, events_pointer );
			events[ i ] = NULL;
		}
	}
}

void twist_gameplay() {
	VDP_clearPlane( BG_A, TRUE );
	VDP_clearPlane( BG_B, TRUE );
	VDP_clearPlane( WINDOW, TRUE );

	JOY_setEventHandler( twist_board_handler );

	u16** frames = SPR_loadAllFrames( &cursor, TILE_USER_INDEX + 16, NULL );
	twist_load_swirl_gfx();

	// Game state
	Board game_state = ( Board ) {
		.board = { 0 },
		.select_anim_palette = { 0 },
		.score = 0,
		.dirty = TRUE,
		.on_screen_cursor = SPR_addSpriteExSafe(
			&cursor,
			0,
			0,
			TILE_ATTR( PAL1, TRUE, FALSE, FALSE ),
			NULL,
			SPR_FLAG_AUTO_VISIBILITY | SPR_FLAG_AUTO_SPRITE_ALLOC | SPR_FLAG_AUTO_TILE_UPLOAD
		)
	};

	twist_init_board( &game_state );

	BoardEvent events[ TWIST_MAX_EVENTS ] = { NULL };

	while( TRUE ) {
		twist_update_cursor( &game_state );
		twist_draw_board( &game_state );
		twist_execute_events( &game_state, events );

		SPR_update();
		SYS_doVBlankProcess();
	}

	MEM_free( frames );
}