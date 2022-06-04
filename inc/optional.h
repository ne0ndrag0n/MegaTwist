#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <genesis.h>

typedef struct {
	s8 present;
	Vect2D_s16 value;
} Optional_Vect2D_s16;

typedef struct {
	s8 present;
	s8 value;
} Optional_s8;

#endif