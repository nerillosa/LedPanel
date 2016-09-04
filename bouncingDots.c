// bouncingDots.c
// Running this program shows a bunch of multicolor dots bouncing around.
// After installing bcm2835, you can build and run this with:
// gcc -o bouncingDots -l rt bouncingDots.c lcdshapes.c -l bcm2835
// sudo ./bouncingDots

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"
#include "lcdfonts.h"

#define MOVE_INTERVAL  100  // time in milliseconds between each move
#define NUMBER_DOTS    20

void paintCanvas(uint8_t *canvas);
void initDotSprites();
void showDotSprite(struct dotSprite *dot, uint8_t *canvas);

static struct dotSprite {
   Point point;
   short dx,dy;
} *dotSprites = NULL;

static struct timeval saved, now;

int main(int argc, char **argv)
{
	if (gpio_init())
		return 1;
	uint8_t *canvas = (uint8_t *)calloc(PANEL_SIZE, sizeof(uint8_t));
	srand(time(NULL));
	gettimeofday(&saved, NULL); //start time
	while (true) //infinite loop
	{
		displayRowInit();
		paintCanvas(canvas);
		updateRows(canvas);
	}
}

void paintCanvas(uint8_t *canvas){
	if(dotSprites == NULL){
		initDotSprites();
	}
	gettimeofday(&now, NULL);

        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		int i;
		for(i=0;i<NUMBER_DOTS;i++){
			showDotSprite(dotSprites + i, canvas);
		}
	}
}

void initDotSprites(){
	dotSprites = (struct dotSprite *)malloc(sizeof(struct dotSprite)* NUMBER_DOTS);
	int i;
	for(i=0;i<NUMBER_DOTS;i++){
		(dotSprites+i)->point.x = rand()%(TOTAL_NUMBER_COLUMNS-2)+1;
		(dotSprites+i)->point.y=rand()%(NUMBER_ROWS-2)+1;
		(dotSprites+i)->point.c = (rand()%7) + 1;
		(dotSprites+i)->dx = rand()%2 ? 1 : -1;
		(dotSprites+i)->dy = rand()%2 ? 1 : -1;
	}
}

void showDotSprite(struct dotSprite *dot, uint8_t *canvas){
		drawPoint(dot->point, canvas);

		if(dot->point.x == 0 || dot->point.x == TOTAL_NUMBER_COLUMNS-1){
			dot->dx *= -1;
		}
		if(dot->point.y == 0 || dot->point.y == NUMBER_ROWS-1){
			dot->dy *= -1;
		}

		dot->point.x += dot->dx;
		dot->point.y += dot->dy;
}
