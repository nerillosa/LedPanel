// rssLatestFeeds.c
// Running this program shows the 20 most recent FOX,REUTERS,CNN, etc. politics feeds titles scrolling from right to left.
// After installing bcm2835 and libcurl, you can build and run this with:
// gcc -o rssLatestFeeds rssLatestFeeds.c lcdshapes.c parseRss.c -lrt -lbcm2835 -lcurl
// sudo ./rssFeedTicker

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"
#include "lcdfonts.h"
#include "parseRss.h"

#define MOVE_INTERVAL  30  // time in milliseconds between each move

void paintCanvas(uint8_t *canvas);

struct timeval saved, now;

char **titles; // pointer to an array of strings
char *mesg;
char agencia[10];
char hora[10];
int strlength;
struct item *allItems = NULL;
//int allItemsSize;

int main(int argc, char **argv)
{
 	if (gpio_init())
		return 1;

	uint8_t *canvas = (uint8_t *)calloc(PANEL_SIZE, sizeof(uint8_t));
	gettimeofday(&saved, NULL); //start time
	getLatestItems(&allItems);
	mesg = allItems[0].title;

	strcpy(agencia,allItems[0].agency);
	strncpy(hora, allItems[0].pubDate +17, 5);
	strcat(hora, " GMT");
	strlength = strlen(mesg);

	while (true) //infinite loop
	{
		displayRowInit();
		paintCanvas(canvas);
		updateRows(canvas);
	}
}

void paintCanvas(uint8_t *canvas){
	static int mesgCounter = 0;
	static int lineCounter = 0;
	static int flip = -1;
	static int moveOffset = NUMBER_PANELS * NUMBER_COLUMNS_PER_PANEL;
	static int agencyCounter = 0;
	static color clr = blue;

	gettimeofday(&now, NULL);

        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		if(--moveOffset == -1*LETTER_WIDTH*strlength){ //message repeated itself
			moveOffset = TOTAL_NUMBER_COLUMNS;
			if(++mesgCounter == 1) { // message repeated 1 times. time for next title
				mesgCounter = 0;
				if(++clr == 8) clr = blue;
				if(clr == red) clr++; //first row is always red
				if(++lineCounter == NUM_TITLES) { //refresh feed after all titles have scrolled
					lineCounter = 0;
					getLatestItems(&allItems);
				}
				mesg = allItems[lineCounter].title;
				strlength = strlen(mesg);

				strcpy(agencia,allItems[lineCounter].agency);
				strncpy(hora, allItems[lineCounter].pubDate +17, 5);
				strcat(hora, " GMT");
				agencyCounter = 0;
				flip *= -1;
			}
		}
		int i;

		if(++agencyCounter%40 == 0) flip *= -1;
		if(flip > 0)
			for(i=0;i<strlen(agencia);i++){
				drawLetter(*(agencia+i), LETTER_WIDTH*i, 0, red, canvas);
			}
		else
			for(i=0;i<strlen(hora);i++){
				drawLetter(*(hora+i), LETTER_WIDTH*i, 0, red, canvas);
			}

        	for(i=0;i<strlength;i++){
              		drawLetter(*(mesg+i), moveOffset + LETTER_WIDTH*i, 9, clr, canvas);
	      	}
	}
}
