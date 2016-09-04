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

static void paintCanvas(uint8_t *canvas);

static int picPointsSize;
static bool alterPicPoints();
static void initPicPoints();
static void randomizePicPoints();
static struct timeval saved, now;
static char *mesg = "HI THERE!";

static struct pointPair {
   Point current, final;
} *picPoints = NULL;


int main(int argc, char **argv)
{
	if (gpio_init())
		return 1;
	uint8_t *canvas = (uint8_t *)calloc(PANEL_SIZE, sizeof(uint8_t));
	gettimeofday(&saved, NULL); //start time
	srand(time(NULL));
	while (true) //infinite loop
	{
		displayRowInit();
		paintCanvas(canvas);
		updateRows(canvas);
	}
}

void paintCanvas(uint8_t *canvas){
	static int state = 0;

	if(picPoints == NULL){ //one time initialization
		initPicPoints();
	}
	gettimeofday(&now, NULL);
        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		int i;
	        for(i=0;i<picPointsSize;i++){
        	        drawPoint(picPoints[i].current, canvas);
        	}
		if(!alterPicPoints()){
			if(++state == 20){ // show the unaltered message for 20 MOVE_INTERVALs (~2 sec)
				state = 0;
				randomizePicPoints(); //re-randomize screen
			}
		}
	}
}

bool alterPicPoints(){
	int i,j;
	bool altered = false;
	for(i=0;i<picPointsSize;i++){
		struct pointPair *picPoint = picPoints + i;
		if(picPoint->current.x == picPoint->final.x && picPoint->current.y == picPoint->final.y){
			picPoint->current.c = picPoint->final.c;
			continue;
		}
		altered = true;
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
	return altered;
}

void initPicPoints(){
	int i,j;
	int slen = strlen(mesg);
	picPoints = (struct pointPair *) malloc (PANEL_SIZE * sizeof(struct pointPair));

	uint8_t canvas[PANEL_SIZE];
	memset(canvas, 0, PANEL_SIZE);

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
	picPointsSize = j;
	randomizePicPoints();
}

void randomizePicPoints(){
	uint8_t chosen[PANEL_SIZE];
	memset(chosen, 0, PANEL_SIZE);
	int r, i;
	for(i=0;i<picPointsSize;i++){
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
}
