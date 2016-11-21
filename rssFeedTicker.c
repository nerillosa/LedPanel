// rssFeedTicker.c
// Running this program shows FOX,REUTERS,CNN, etc. politics feeds titles scrolling from right to left.
// After installing bcm2835 and libcurl, you can build and run this with:
// gcc -o rssFeedTicker rssFeedTicker.c lcdshapes.c parseXml.c -lrt -lbcm2835 -lcurl -lpthread
// sudo ./rssFeedTicker

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"
#include "lcdfonts.h"
#include "parseXml.h"

#define MOVE_INTERVAL  30  // time in milliseconds between each move

struct newsAgency{
 char name[10];
 char *url;
};

static struct urls {
 struct newsAgency agency;
 struct urls *next;
} *foxurls = NULL;

void paintCanvas(uint8_t *canvas);
void getUrls();

struct timeval saved, now;

char **titles; // pointer to an array of strings
char *mesg;
struct news newsTitles;
int numLines;
int strlength;
struct newsAgency politics[] = {
                               {"FOX","http://feeds.foxnews.com/foxnews/politics?format=xml"},
                               {"NYTIMES","http://rss.nytimes.com/services/xml/rss/nyt/Politics.xml"},
                               {"W.POST","http://feeds.washingtonpost.com/rss/rss_powerpost"},
                               {"CNBC","http://www.cnbc.com/id/10000113/device/rss/rss.html"},
                               {"ABC","http://feeds.abcnews.com/abcnews/politicsheadlines"},
                               {"REUTERS","http://feeds.reuters.com/Reuters/PoliticsNews"},
                               {"CNN","http://rss.cnn.com/rss/cnn_allpolitics.rss"},
				};

int main(int argc, char **argv)
{
	if (gpio_init())
		return 1;

	getUrls();

	newsTitles.titles = NULL;

	while(refreshFeed(foxurls -> agency.url, &newsTitles)){
		foxurls = foxurls -> next;
	}

	numLines = newsTitles.size;
	titles = (char **)newsTitles.titles;
        mesg = titles[0];
	strlength = strlen(mesg);

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
					foxurls = foxurls -> next; //next category
					while(refreshFeed(foxurls -> agency.url, &newsTitles)){ //advance to next url if refresh news category fails
						foxurls = foxurls -> next;
					}
					numLines = newsTitles.size;
					titles = (char **)newsTitles.titles;
				}
				mesg = titles[lineCounter];
				strlength = strlen(mesg);
			}
		}
		int i=0;
		char *namee = foxurls -> agency.name;
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
