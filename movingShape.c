// movingSmileyFace.c
//
// Uses bcm2835, a fast GPIO C library by Mike McCauley. I just use the basic output functionality.
// To install : http://www.raspberry-projects.com/pi/programming-in-c/io-pins/bcm2835-by-mike-mccauley
//
// Running this program shows a moving smiley, but this can be any user defined shape
//
// The primitives for drawing shapes are located in lcdshapes.c
//
// After installing bcm2835, you can build and run this with:
// gcc -o movingShape -l rt movingShape.c lcdshapes.c -l bcm2835
// sudo ./movingShape
//
// Author: Neri Llosa (nerillosa@gmail.com)
// Copyright (C) 2016 Neri Llosa

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"

void paintCanvas(uint16_t count, uint16_t *moveOffset, uint8_t *displayArray);

int main(int argc, char **argv)
{
	if (gpio_init())
		return 1;

	uint8_t *canvas = (uint8_t *)malloc(PANEL_SIZE * sizeof(uint8_t));

	uint16_t count = 0, moveOffset = 0;

	while (1) //infinite loop
	{
		displayRowInit(count);
		paintCanvas(count,&moveOffset, canvas);
		updateRows(count, canvas);
		displayRowEnd();
		count++;
	}
}

void paintCanvas(uint16_t count, uint16_t *moveOffset, uint8_t *displayArray){
	if(count%128 == 0){ // shift once to the left every 128 cycles.
		memset(displayArray, 0, PANEL_SIZE);
		if(++*moveOffset == (TOTAL_NUMBER_COLUMNS))
			*moveOffset = 0;
		drawSmileyFace(*moveOffset + 7, 8, 15, displayArray);
	}
}

