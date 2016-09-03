// starBurstMessage.c
// This program shows a random number of dots of various colors
// transforming into a message.
// After installing bcm2835, you can build and run this with:
// gcc -o starBurstMessage -l rt starBurstMessage.c lcdshapes.c -l bcm2835
// sudo ./starBurstMessage

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"
#include "lcdfonts.h"

#define MOVE_INTERVAL  100  // time in milliseconds between each move

struct pointPair {
	Point current, final;
};

void paintCanvas(uint8_t *canvas, struct pointPair *picPoints, int picPointsSize);
void alterPicPoints(struct pointPair *picPoints, int picPointsSize);

struct timeval saved, now;
char *mesg = "HI THERE!";

int main(int argc, char **argv)
{

	struct pointPair picPoints[PANEL_SIZE];

	if (gpio_init())
		return 1;

	uint8_t *canvas = (uint8_t *)calloc(PANEL_SIZE, sizeof(uint8_t));
	gettimeofday(&saved, NULL); //start time

	int i,j;
	int slen = strlen(mesg);

	for(i=0;i<slen;i++){ //Draw the message in the canvas
		drawLetter(*(mesg+i), LETTER_WIDTH*i, 5, green, canvas);
	}

	for(i=0,j=0;i<PANEL_SIZE;i++){ //save the message as an array of points
 		if(canvas[i]){
			picPoints[j].final.x = i%TOTAL_NUMBER_COLUMNS;
			picPoints[j].final.y = i/TOTAL_NUMBER_COLUMNS;
			picPoints[j].final.c = green;
			j++;
		}
	}

	int picPointsSize = j;
	memset(canvas, 0, PANEL_SIZE); // clear the canvas

	uint8_t chosen[PANEL_SIZE];
	memset(chosen, 0, PANEL_SIZE);

	srand(time(NULL));

	int r;
	for(i=0;i<picPointsSize;i++){ // create a random array of points with same size as message points
		while(true){
			r = rand()%PANEL_SIZE;
			if(!chosen[r]){
				chosen[r]=1;
				picPoints[i].current.x = r%TOTAL_NUMBER_COLUMNS;
				picPoints[i].current.y = r/TOTAL_NUMBER_COLUMNS;
				picPoints[i].current.c = (rand()%7) + 1;
				break;
			}
		}
        }

	while (true) //infinite loop
	{
		displayRowInit();
		paintCanvas(canvas, picPoints, picPointsSize);
		updateRows(canvas);
	}
}

void paintCanvas(uint8_t *canvas, struct pointPair *picPoints, int picPointsSize){
	static int moveOffset = NUMBER_PANELS * NUMBER_COLUMNS_PER_PANEL;
	gettimeofday(&now, NULL);
	int strlength = strlen(mesg);

        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		int i=0;
	        for(i=0;i<picPointsSize;i++){
        	        drawPixel2(picPoints[i].current, canvas);
        	}
		alterPicPoints(picPoints, picPointsSize);
	}
}

void alterPicPoints(struct pointPair *picPoints, int picPointsSize){
	int i,j;
	for(i=0;i<picPointsSize;i++){
		struct pointPair *picPoint = picPoints + i;
		if(picPoint->current.x == picPoint->final.x && picPoint->current.y == picPoint->final.y){
			picPoint->current.c = picPoint->final.c;
			continue;
		}
		j = rand()%5;
		if(j){ // move the x 4 times more than the y
			if(picPoint->current.x != picPoint->final.x){
				if(picPoint->current.x < picPoint->final.x)
					picPoint->current.x++;
				else
					picPoint->current.x--;
			}
		}else{
			if(picPoint->current.y != picPoint->final.y){
				if(picPoint->current.y < picPoint->final.y)
					picPoint->current.y++;
				else
					picPoint->current.y--;
			}
		}
	}
}
