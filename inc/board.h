#ifndef BOARD_H
#define BOARD_H

#include <genesis.h>

#define TWIST_SWIRLS_X				15
#define TWIST_SWIRLS_Y				14
#define TWIST_MAX_EVENTS			64
#define TWIST_ARGS_PER_EVENT		3
#define TWIST_BYTES_PER_ARG			16
#define TWIST_BOARD_INDEX( x, y )	( TWIST_SWIRLS_X * y + x )

#define USED_BUTTONS	( BUTTON_LEFT | BUTTON_RIGHT | BUTTON_UP | BUTTON_DOWN )

// Swirl.type
#define NO_SWIRL		-1

// Swirl.palette_animation
#define NO_ANIMATION    0
#define TO_SELECTED     1
#define TO_UNSELECTED   -1

#define NO_VALUE		-1

typedef struct {
	s8 type;
	s8 selected;
	s8 palette_animation;
} Swirl;

typedef struct {
	Swirl board[ TWIST_SWIRLS_X * TWIST_SWIRLS_Y ];
	u16 select_anim_palette[ 16 ];
	s32 score;
	s8 dirty;
	Sprite* on_screen_cursor;
	void* on_palette_anim;
} Board;

typedef struct {
	s8 in_use;
	unsigned char data[ TWIST_BYTES_PER_ARG ];
} ClosureArguments;

typedef void (*BoardEvent)( Board*, ClosureArguments*, void* );

typedef struct {
	BoardEvent function;
	ClosureArguments* arguments;
} Closure;

typedef struct {
	Closure events[ TWIST_MAX_EVENTS ];
	Closure deferred_pool[ TWIST_MAX_EVENTS ];
	ClosureArguments arg_pool[ TWIST_MAX_EVENTS * TWIST_ARGS_PER_EVENT ];
} EventLoop;

void twist_gameplay();

#endif