// scrambledMessage.c
// Running this program shows a bunch of multicolor letters around around
// until they coalesce into a message.
// After installing bcm2835, you can build and run this with:
// gcc -o scrambledMessage -l rt scrambledMessage.c lcdshapes.c -l bcm2835
// sudo ./scrambledMessage

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"
#include "lcdfonts.h"

#define MOVE_INTERVAL  50  // time in milliseconds between each move

static struct timeval saved, now;
static struct letterSprite {
   Point point;
   char  val;
   short dx,dy;
} *letterSprites = NULL;

static char *mesg = "HI THERE!";
static int  mesg_length;
static void paintCanvas(uint8_t *canvas);
static void initLetterSprite(struct letterSprite *sprite, char value);
static void initLetterSprites();
static void showLetterSprite(struct letterSprite *letter, uint8_t *canvas);
static bool lettersIntersect(Point vertixA, Point vertixB);

int main(int argc, char **argv)
{
	if (gpio_init())
		return 1;
	uint8_t *canvas = (uint8_t *)calloc(PANEL_SIZE, sizeof(uint8_t));
	srand(time(NULL));
	mesg_length = strlen(mesg);
	gettimeofday(&saved, NULL); //start time
	while (true) //infinite loop
	{
		displayRowInit();
		paintCanvas(canvas);
		updateRows(canvas);
	}
}

void paintCanvas(uint8_t *canvas){
	if(letterSprites == NULL){
		initLetterSprites();
	}
	gettimeofday(&now, NULL);
        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		int i;
		for(i=0;i<mesg_length;i++){
			Point p = {LETTER_WIDTH*i, 5};
			if(lettersIntersect(p, letterSprites[i].point)){
				letterSprites[i].point.x = LETTER_WIDTH*i;
				letterSprites[i].point.y = 5;
				letterSprites[i].dx=0;
				letterSprites[i].dy=0;
			}
			showLetterSprite(letterSprites + i, canvas);
		}
	}
}

void initLetterSprite(struct letterSprite *sprite, char value){
		sprite->point.x = rand()%(TOTAL_NUMBER_COLUMNS+5)-5;
		sprite->point.y = rand()%(NUMBER_ROWS+6)-6;
		sprite->point.c = (rand()%7) + 1;
		sprite->dx = rand()%2 ? 1 : -1;
		sprite->dy = rand()%2 ? 1 : -1;
		if(value) sprite->val = value;
}

void initLetterSprites(){
	int strlength = strlen(mesg);
	letterSprites = (struct letterSprite *)malloc(sizeof(struct letterSprite)* mesg_length);
	int i;
	for(i=0;i<mesg_length;i++){
		initLetterSprite(letterSprites+i, mesg[i]);
	}
}

void showLetterSprite(struct letterSprite *letter, uint8_t *canvas){
		drawLetterP(letter->val,letter->point, canvas);
		if(letter->point.x < -5 || letter->point.x == TOTAL_NUMBER_COLUMNS ||
			letter->point.y < -6 || letter->point.y == NUMBER_ROWS){
				initLetterSprite(letter, 0);
				return;
		}
		letter->point.x += letter->dx;
		letter->point.y += letter->dy;
}

bool lettersIntersect(Point vertixA, Point vertixB){
	return (vertixA.x < (vertixB.x + LETTER_WIDTH) && (vertixA.x + LETTER_WIDTH) > vertixB.x &&
		vertixA.y < (vertixB.y + LETTER_HEIGHT) && (vertixA.x + LETTER_HEIGHT) > vertixB.y );
}
