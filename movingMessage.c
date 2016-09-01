// movingMessage.c
// Running this program shows a text message moving from right to left. 
// The message can be defined as an input parameter.
// Messages can have a maximum length of 200 characters.
// The function for drawing letters is located in lcdshapes.c
//
// After installing bcm2835, you can build and run this with:
// gcc -o movingMessage -l rt movingMessage.c lcdshapes.c -l bcm2835
// sudo ./movingMessage "HI THERE!"

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"
#include "lcdfonts.h"

#define MOVE_INTERVAL  50  // time in milliseconds between each move
#define MAX_MESG_LENGTH 200

void paintCanvas(uint8_t *canvas);

struct timeval saved, now;

char mesg[MAX_MESG_LENGTH] = "GOOD DAY TO YOU!"; //default message if none given as a parameter

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
	static int moveOffset = NUMBER_PANELS * NUMBER_COLUMNS_PER_PANEL;
	static color clr = blue;
	gettimeofday(&now, NULL);
	int strlength = strlen(mesg);

        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		if(--moveOffset == -1*LETTER_WIDTH*strlength){
			moveOffset = TOTAL_NUMBER_COLUMNS;
			if(++clr == 8) clr = blue;
		}
		int i=0;
        	for(i=0;i<strlength;i++){
              		drawLetter(*(mesg+i), moveOffset + LETTER_WIDTH*i, 5, clr, canvas);
	      	}
	}
}

