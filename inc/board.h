#ifndef BOARD_H
#define BOARD_H

#include <genesis.h>

#define TWIST_SWIRLS_X		15
#define TWIST_SWIRLS_Y		15
#define TWIST_MAX_EVENTS	64

#define USED_BUTTONS	( BUTTON_LEFT | BUTTON_RIGHT | BUTTON_UP | BUTTON_DOWN )

// Swirl.type
#define NO_SWIRL		-1

// PaletteAnimation.type
#define NO_ANIMATION    0
#define TO_SELECTED     1
#define TO_UNSELECTED   -1

typedef struct {
	s8 type;
	s16 remaining_steps;		// To or from 0x0FFF as determined by type
} PaletteAnimation;

typedef struct {
	s8 type;
	s8 selected;
	PaletteAnimation palette_animation;
} Swirl;

typedef struct Board {
	Swirl board[ TWIST_SWIRLS_X * TWIST_SWIRLS_Y ];
	u16 select_anim_palette[ 16 ];
	s32 score;
	s8 dirty;
	Sprite* on_screen_cursor;
} Board;

typedef void (*BoardEvent)( Board*, void* );

void twist_gameplay();

#endif