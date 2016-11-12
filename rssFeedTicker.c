// rssFeedTicker.c
// Running this program shows FOX Rss news feeds scrolling from right to left.
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

struct news {
  void *titles;
  int size;
};

struct newsAgency{
 char name[10];
 char *url;
};

static struct urls {
 struct newsAgency agency;
 struct urls *next;
} *foxurls = NULL;

void getNews(struct news *newsTitles);
void paintCanvas(uint8_t *canvas);
void getUrls();

struct timeval saved, now;

char **titles; // pointer to an array of strings
char *mesg;
struct news newsTitles;
int numLines;
int strlength;
struct newsAgency politics[] = {{"FOX","http://feeds.foxnews.com/foxnews/politics?format=xml"},{"REUTER","http://feeds.reuters.com/Reuters/PoliticsNews"}};
//char* categories[] = {"health", "business", "national", "world", "latest", "politics", "scitech", "entertainment"};

int main(int argc, char **argv)
{
	if (gpio_init())
		return 1;

	getUrls();

	while(refreshFeed(foxurls -> agency.url)){
		foxurls = foxurls -> next;
	}

	newsTitles.titles = NULL;
	getNews(&newsTitles);
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
					while(refreshFeed(foxurls -> agency.url)){ //advance to next url if refresh news category fails
						foxurls = foxurls -> next;
					}
					getNews(&newsTitles);
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
		sprintf(&letra[1], "%d", numLines - lineCounter);
		int k = 0;
		for(k=0;k<strlen(letra);k++,i++){
			drawLetter(*(letra+k), LETTER_WIDTH*i, 0, red, canvas);
		}
        	for(i=0;i<strlength;i++){
              		drawLetter(*(mesg+i), moveOffset + LETTER_WIDTH*i, 9, clr, canvas);
	      	}
	}
}

void getNews(struct news *newsTitles){
	int j = 0;
	if(newsTitles -> titles != NULL){ //free all the allocated memory
		char **titless = (char **)(newsTitles -> titles);
		for(j=0;j<newsTitles -> size; j++){
			free(titless[j]);
		}
		free(newsTitles -> titles);
	}
	char* fileName = "feed.txt";
	FILE* file = fopen(fileName, "r");
        if (file == NULL){
                printf("Could not open feed file \n");
                return;
        }

	char line[256];
	memset(line, 0, 256);

	char **news = malloc(100 * sizeof(char*)); // 100 titles of 256 chars each
	int nlines = 0;

	while (fgets(line, sizeof(line), file)) {
		int i = 0;
		while(line[i]){
			line[i] = toupper(line[i]);
			i++;
		}
		news[nlines] = malloc(strlen(line) +1);
		strcpy(news[nlines++], line);
	}

	newsTitles -> titles = news;
	newsTitles -> size = nlines;

	fclose(file);
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
