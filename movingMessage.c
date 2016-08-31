// movingMessage.c
//
// Uses bcm2835, a fast GPIO C library by Mike McCauley. I just use the basic output functionality.
// To install : http://www.raspberry-projects.com/pi/programming-in-c/io-pins/bcm2835-by-mike-mccauley
//
// Running this program shows a moving text message. The message can be defined as an input parameter.
// Messages can have a maximum length of 200 characters.
// The function for drawing letters is located in lcdshapes.c
//
// After installing bcm2835, you can build and run this with:
// gcc -o movingMessage -l rt movingMessage.c lcdshapes.c -l bcm2835
// sudo ./movingMessage "HI THERE!"
//
// Author: Neri Llosa (nerillosa@gmail.com)
// Copyright (C) 2016 Neri Llosa

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"
#include "lcdfonts.h"

#define MOVE_INTERVAL  150  // time in milliseconds between each move
#define MAX_MESG_LENGTH 200

void paintCanvas(uint8_t *canvas);

struct timeval saved, now;

char mesg[MAX_MESG_LENGTH] = "DO YOU REMEMBER HOW IT USED TO BE ?"; //default message if none given as a parameter

int main(int argc, char **argv)
{
	if(argc > 1){
		snprintf(mesg, MAX_MESG_LENGTH, "%s", argv[1]);
	}

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
	static int moveOffset = 64;
	gettimeofday(&now, NULL);
	int strlength = strlen(mesg);

        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		if(--moveOffset == -1*LETTER_WIDTH*strlength)
			moveOffset = TOTAL_NUMBER_COLUMNS;
		int i=0;
        	for(i=0;i<strlength;i++){
              		drawLetter(*(mesg+i), moveOffset + LETTER_WIDTH*i, 5, blue, canvas);
	      	}
	}
}

