#include <genesis.h>
#include <maths.h>
#include "board.h"
#include "optional.h"
#include "resources.h"
#include "swirls.h"

static u16 joyDump = 0;

static void twist_init_board( Board* game_state ) {
	memcpy( game_state->select_anim_palette, twist_swirl1.palette->data, 32 );
	SPR_setVRAMTileIndex( game_state->on_screen_cursor, TILE_USER_INDEX + 16 );

	for( int y = 0; y < TWIST_SWIRLS_Y; y++ ) {
		for( int x = 0; x < TWIST_SWIRLS_X; x++ ) {
			game_state->board[ TWIST_SWIRLS_X * y + x ] = ( Swirl ) {
				.type = random() % 4,
				.selected = FALSE,
				.palette_animation = NO_ANIMATION
			};
		}
	}
}

static Optional_Vect2D_s16 twist_find_any_selected( Swirl* board ) {
	Optional_Vect2D_s16 result = ( Optional_Vect2D_s16 ) {
		.present = FALSE,
		.value = ( Vect2D_s16 ) {
			.x = 0,
			.y = 0
		}
	};

	for( int y = 0; y < TWIST_SWIRLS_Y; y++ ) {
		for( int x = 0; x < TWIST_SWIRLS_X; x++ ) {
			if( board[ TWIST_SWIRLS_X * y + x ].selected ) {
				result.present = TRUE;
				result.value.x = x;
				result.value.y = y;

				return result;
			}
		}
	}

	return result;
}

static Optional_Vect2D_s16 twist_find_any_animating( Swirl* board ) {
	for( int y = 0; y < TWIST_SWIRLS_Y; y++ ) {
		for( int x = 0; x < TWIST_SWIRLS_X; x++ ) {
			if( board[ TWIST_SWIRLS_X * y + x ].palette_animation != NO_ANIMATION ) {
				return ( Optional_Vect2D_s16 ) {
					.present = TRUE,
					.value = ( Vect2D_s16 ) { .x = x, .y = y }
				};
			}
		}
	}

	return ( Optional_Vect2D_s16 ) {
		.present = FALSE,
		.value = ( Vect2D_s16 ) {
			.x = 0,
			.y = 0
		}
	};
}

static void twist_execute_events( Board* game_state, EventLoop* event_loop ) {
	for( int i = 0; i < TWIST_MAX_EVENTS; i++ ) {
		if( event_loop->events[ i ].function ) {
			event_loop->events[ i ].function( game_state, event_loop->events[ i ].arguments, event_loop );

			event_loop->events[ i ].function = NULL;
			if( event_loop->events[ i ].arguments ) {
				event_loop->events[ i ].arguments->in_use = FALSE;
			}
		}
	}
}

static void twist_enqueue_event( Closure* staged, EventLoop* event_loop ) {
	// Full queue case
	if( event_loop->events[ TWIST_MAX_EVENTS - 1 ].function ) {
		VDP_drawText( "Event queue full", 2, 2 );
		VDP_drawText( "System Halted", 2, 3 );
		while( 1 );
	}

	// Empty queue case
	if( !event_loop->events[ 0 ].function ) {
		event_loop->events[ 0 ] = *staged;
		return;
	}

	// Slot this item after the last executed event
	for( int i = TWIST_MAX_EVENTS - 2; i >= 0; i-- ) {
		if( event_loop->events[ i ].function ) {
			event_loop->events[ i + 1 ] = *staged;
			return;
		}
	}
}

static Closure* twist_defer_event( Closure* staged, EventLoop* event_loop ) {
	for( int i = 0; i < TWIST_MAX_EVENTS; i++ ) {
		if( !event_loop->deferred_pool[ i ].function ) {
			event_loop->deferred_pool[ i ] = *staged;
			return event_loop->deferred_pool + i;
		}
	}

	VDP_drawText( "Deferred pool full", 2, 2 );
	VDP_drawText( "System Halted", 2, 3 );
	while( 1 );
}

static void twist_enqueue_deferred_event( Closure* staged, EventLoop* event_loop ) {
	twist_enqueue_event( staged, event_loop );

	staged->function = NULL;
}

static void twist_draw_board( Board* game_state, EventLoop* event_loop ) {
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
							game_state->board[ index ].selected ? PAL2 : PAL1,
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
							PAL1,
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

	// Play animations (typically only one type of swirl is selected and we update it all at once)
	Optional_Vect2D_s16 animating = twist_find_any_animating( game_state->board );
	if( animating.present ) {
		// What segment of the palette is being updated?
		u8 index = ( game_state->board[ TWIST_BOARD_INDEX( animating.value.x, animating.value.y ) ].type * 3 ) + 1;

		// What is the expected final result of the calculation?
		u16 endpoints[ 3 ] = { 0 };
		s16 direction;
		if( game_state->board[ TWIST_BOARD_INDEX( animating.value.x, animating.value.y ) ].palette_animation == TO_SELECTED ) {
			endpoints[ 0 ] =
			endpoints[ 1 ] =
			endpoints[ 2 ] =
				0x0FFF;

			direction = 1;
		} else {
			// it will be TO_UNSELECTED since find_any_animating returns a non-NO_ANIMATION status
			endpoints[ 0 ] = twist_swirl1.palette->data[ index ];
			endpoints[ 1 ] = twist_swirl1.palette->data[ index + 1 ];
			endpoints[ 2 ] = twist_swirl1.palette->data[ index + 2 ];

			direction = -1;
		}

		// Calculate the new palette value
		s8 no_work_done = TRUE;
		for( int i = 0; i < 3; i++ ) {
			if( game_state->select_anim_palette[ index + i ] != endpoints[ i ] ) {
				no_work_done = FALSE;

				u32 col = game_state->select_anim_palette[ index + i ];
				col += ( direction * 330 );

				// clamp
				if( direction == 1 && col > endpoints[ i ] ) {
					col = endpoints[ i ];
				}

				if( direction == -1 && col < endpoints[ i ] ) {
					col = endpoints[ i ];
				}

				game_state->select_anim_palette[ index + i ] = ( u16 ) col;
			}
		}

		// If we just did nothing, stage the on_palette_anim callback & remove all animations
		// otherwise, upload the palette and keep on going
		if( no_work_done ) {
			for( int i = 0; i < TWIST_SWIRLS_X * TWIST_SWIRLS_Y; i++ ) {
				game_state->board[ i ].palette_animation = NO_ANIMATION;
			}

			if( game_state->on_palette_anim ) {
				twist_enqueue_deferred_event( game_state->on_palette_anim, event_loop );
				game_state->on_palette_anim = NULL;
			}
		} else {
			// If there's more work to be done, send the palette and keep on truckin'
			PAL_setColors( 32, game_state->select_anim_palette, 16, DMA_QUEUE_COPY );
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

static void twist_board_handler( u16 joy, u16 changed, u16 state ) {
	switch ( joy ) {
		case JOY_1: {
			joyDump = state;
		}
	}
}

static ClosureArguments* twist_get_arg_block( EventLoop* event_loop ) {
	for( int i = 0; i < TWIST_MAX_EVENTS * TWIST_ARGS_PER_EVENT; i++ ) {
		if( !event_loop->arg_pool[ i ].in_use ) {
			event_loop->arg_pool[ i ].in_use = TRUE;
			return event_loop->arg_pool + i;
		}
	}

	// Argument pool full
	VDP_drawText( "Argument pool full", 2, 2 );
	VDP_drawText( "System Halted", 2, 3 );
	while( 1 );
}

static void twist_select( Swirl* board, s8 x, s8 y, s8 selected, s8 base_type ) {
	// Base cases
	if( x >= TWIST_SWIRLS_X || y >= TWIST_SWIRLS_Y || x < 0 || y < 0 ) {
		return;
	}

	if( board[ TWIST_SWIRLS_X * y + x ].type != base_type || board[ TWIST_SWIRLS_X * y + x ].selected == selected ) {
		return;
	}

	board[ TWIST_SWIRLS_X * y + x ].selected = selected;
	board[ TWIST_SWIRLS_X * y + x ].palette_animation = selected == TRUE ? TO_SELECTED : TO_UNSELECTED;

	twist_select( board, x + 1, y, selected, base_type );
	twist_select( board, x - 1, y, selected, base_type );
	twist_select( board, x, y + 1, selected, base_type );
	twist_select( board, x, y - 1, selected, base_type );
}

static void twist_on_swirl_selected( Board* game_state, ClosureArguments* arguments, void* event_loop_ref ) {
	Vect2D_s16 selected;
	memcpy( &selected, arguments->data, sizeof( Vect2D_s16 ) );

	s16 selected_index = TWIST_SWIRLS_X * selected.y + selected.x;
	// If this is already selected, unselect it
	if( game_state->board[ selected_index ].selected ) {
		twist_select( game_state->board, selected.x, selected.y, FALSE, game_state->board[ selected_index ].type );
	} else {
		// If this is not selected, check and see if anything else is selected.
		// If those items ARE selected, deselect them, then flood select.
		Optional_Vect2D_s16 any_selected = twist_find_any_selected( game_state->board );

		if( any_selected.present ) {
			// There are other swirls selected, so deselect them, then defer this function.
			s16 any_selected_index = TWIST_SWIRLS_X * any_selected.value.y + any_selected.value.x;
			twist_select( game_state->board, any_selected.value.x, any_selected.value.y, FALSE, game_state->board[ any_selected_index ].type );

			// That found swirl now gets an animation_callback attached to it (they all finish at once)
			// and will just come back in here after the animation is done.
			ClosureArguments* next_arguments = twist_get_arg_block( event_loop_ref );
			memcpy( next_arguments, arguments, sizeof( ClosureArguments ) );
			Closure closure = ( Closure ) { .function = twist_on_swirl_selected, .arguments = next_arguments };
			game_state->on_palette_anim = twist_defer_event( &closure, event_loop_ref );
		} else {
			// There are no other swirls selected, so simply select them (animation gets staged as part of changing selection status)
			twist_select( game_state->board, selected.x, selected.y, TRUE, game_state->board[ selected_index ].type );
		}
	}

	game_state->dirty = TRUE;
}

static void twist_update_cursor( Board* game_state, EventLoop* event_loop ) {
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
		} else if( joyDump & BUTTON_A ) {
			// Do nothing if any animation is actively playing
			if( twist_find_any_animating( game_state->board ).present ) {
				return;
			}

			Vect2D_s16 selected = ( Vect2D_s16 ){ .x = currentPosition.x / 16, .y = currentPosition.y / 16 };
			ClosureArguments* arguments = twist_get_arg_block( event_loop );
			memcpy( arguments->data, &selected, sizeof( Vect2D_s16 ) );

			Closure closure = ( Closure ) {
				.function = twist_on_swirl_selected,
				.arguments = arguments
			};
			twist_enqueue_event( &closure, event_loop );
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
	Board* game_state = ( Board* ) MEM_alloc( sizeof( Board ) );
	*game_state = ( Board ) {
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
		),
		.on_palette_anim = NULL
	};

	EventLoop* event_loop = ( EventLoop* ) MEM_alloc( sizeof( EventLoop ) );
	*event_loop = ( EventLoop ) {
		.events = { 0 },
		.deferred_pool = { 0 },
		.arg_pool = { 0 }
	};

	twist_init_board( game_state );

	while( TRUE ) {
		twist_update_cursor( game_state, event_loop );
		twist_draw_board( game_state, event_loop );
		twist_execute_events( game_state, event_loop );

		SPR_update();
		SYS_doVBlankProcess();
	}

	MEM_free( game_state );
	MEM_free( event_loop );
	MEM_free( frames );
}