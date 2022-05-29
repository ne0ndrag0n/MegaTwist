#ifndef BOARD_H
#define BOARD_H

#define TWIST_SWIRLS_X	15
#define TWIST_SWIRLS_Y	15

#define USED_BUTTONS	( BUTTON_LEFT | BUTTON_RIGHT | BUTTON_UP | BUTTON_DOWN )

typedef struct {
	u8 type;
	u8 selected;
} Swirl;

void twist_gameplay();

#endif