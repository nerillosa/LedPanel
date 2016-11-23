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
#define NUM_TITLES 20

void paintCanvas(uint8_t *canvas);

struct timeval saved, now;

char **titles; // pointer to an array of strings
char *mesg;
int numLines;
int strlength;
struct item *allItems = NULL;
int allItemsSize;

int main(int argc, char **argv)
{
 	if (gpio_init())
		return 1;

	getLatestItems(&allItemsSize, &allItems);
	mesg = allItems[0].title;
	strlength = strlen(mesg);
	numLines = NUM_TITLES;


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
	static int mesgCounter = 0;
	static int lineCounter = 0;
	static int moveOffset = NUMBER_PANELS * NUMBER_COLUMNS_PER_PANEL;
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
				if(++lineCounter == numLines) { //refresh feed after all titles have scrolled
					lineCounter = 0;
				}
				mesg = allItems[lineCounter].title;
				strlength = strlen(mesg);
			}
		}
		int i=0;
		char *namee = allItems[lineCounter].agency;
		int namelen = strlen(namee);
        	for(i=0;i<namelen;i++){
              		drawLetter(*(namee+i), LETTER_WIDTH*i, 0, red, canvas);
		}
		char letra[10];
		memset(letra, 0, 10);
		letra[0] = ' ';
		sprintf(&letra[(namelen<7 || numLines-lineCounter<10) ? 1 : 0], "%d", numLines - lineCounter);
		int k = 0;
		for(k=0;k<strlen(letra);k++,i++){
			drawLetter(*(letra+k), LETTER_WIDTH*i, 0, red, canvas);
		}
        	for(i=0;i<strlength;i++){
              		drawLetter(*(mesg+i), moveOffset + LETTER_WIDTH*i, 9, clr, canvas);
	      	}
	}
}
/*
void getUrls(){
	int lenn = sizeof(politics)/sizeof(politics[0]);
	foxurls = (struct urls *)calloc(lenn, sizeof(struct urls));
	int i;
	for(i=0;i<lenn;i++){
		strcpy(foxurls[i].agency.name, politics[i].name);
		foxurls[i].agency.url = strdup(politics[i].url);
		foxurls[i].next = &foxurls[(i+1)%lenn]; // circular linked list
	}
}
*/
