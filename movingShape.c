// movingShape.c
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

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"

#define MOVE_INTERVAL  150  // time in milliseconds between each move

void paintCanvas(uint8_t *canvas);

struct timeval saved, now;

int main(int argc, char **argv)
{
	if (gpio_init())
		return 1;

	uint8_t *canvas = (uint8_t *)calloc(PANEL_SIZE, sizeof(uint8_t));
	gettimeofday(&saved, NULL); //start time

	while (true) //infinite loop
	{
		displayRowInit();
		paintCanvas(canvas);
		updateRows(canvas);
	}
}

void paintCanvas(uint8_t *canvas){
	static int moveOffset = 0;
	gettimeofday(&now, NULL);
        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		if(++moveOffset == (TOTAL_NUMBER_COLUMNS))
			moveOffset = 0;
		drawSmileyFace(moveOffset + 7, 8, 15, canvas); // smile!
	}
}

