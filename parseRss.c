/*
 * C function that gets the latest rss feeds from various news agencies and extracts the titles
 * Uses libcurl open source library to download feed.xml
 * The function accounts that the files downloaded are not necessarily well formed xml.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <time.h>
#include "parseRss.h"

#define OUT 0
#define IN  1
#define MAX_TITLES 100
#define BUFFER_SIZE 8192
#define ITEM_SIZE 64

struct news {
  char agency[10];
  void *items;
  int size;
} *newsItems = NULL;


struct hcodes{
        char *hcode;
        char value;
} codes[] = {{"&#x2014;",'-'},{"&#8211;",'-'},{"&#8212;",'-'},{"&#x2018;",'\''},{"&#x2019;",'\''},{"&#8216;",'\''},{"&#8217;",'\''},{"&#8220;",'"'},{"&#8221;",'"'},{"&#8230;",'~'},{"&#160;",' '}};

struct newsAgency {
	char name[10];
	char *url;
} politics[] =  {
		{"FOX NEWS","http://feeds.foxnews.com/foxnews/politics?format=xml"},
		{"NY TIMES","http://rss.nytimes.com/services/xml/rss/nyt/Politics.xml"},
		{"WSH POST","http://feeds.washingtonpost.com/rss/rss_powerpost"},
		{"CNBC","http://www.cnbc.com/id/10000113/device/rss/rss.html"},
		{"ABC NEWS","http://feeds.abcnews.com/abcnews/politicsheadlines"},
		{"REUTERS","http://feeds.reuters.com/Reuters/PoliticsNews"},
		{"CNN","http://rss.cnn.com/rss/cnn_allpolitics.rss"},

		};

int NUM_SITES = sizeof(politics)/sizeof(politics[0]);
static void download_feed(FILE *dst, const char *src);
static int countTitleWords(char *str);
static void getTitle(char *line);
static int fsize(const char *filename);
static void cleanItem(char *buffer);
static void cleanHtml(char * const line);  //line start is const
static void fillItems(struct item *items);
static char *getContent(char *starttag, char *endtag, char *text);
static int refreshFeed(struct newsAgency newsAgency);
static int compare_pubDates(const void* a, const void* b);
static void cleanRssDateString(char *rssDateString);


//this function gets exported in parseRss.h
void getLatestItems(struct item **allItems){
        int i;
	static int currentItemsCount = 0;
	static int current_item_size = 0;

	if(*allItems == NULL){
 		*allItems = malloc(ITEM_SIZE * sizeof(struct item));
		current_item_size = ITEM_SIZE;
	}else{
	        for(i=0;i<currentItemsCount;i++){
			free((*allItems)[i].title);
		}
		currentItemsCount = 0;
	}

        for(i=0;i<NUM_SITES;i++){
                refreshFeed(politics[i]);
		if(currentItemsCount > current_item_size - newsItems ->size){
			current_item_size += ITEM_SIZE;
			*allItems =  realloc(*allItems, current_item_size * sizeof(struct item));
		}
		fillItems(*allItems + currentItemsCount);
		currentItemsCount += newsItems ->size;
        }

	printf("currentItemsCount:%d\n",currentItemsCount);

	for(i=0;i<currentItemsCount;i++){
		cleanRssDateString((*allItems)[i].pubDate); // attempt to normalize all dates to GMT/UTC time
	}

	qsort(*allItems, currentItemsCount, sizeof(struct item), compare_pubDates); //sort by pubDate descending

	for(i=0;i<NUM_TITLES;i++){
		printf("%s::",(*allItems)[i].title);
		printf("%s::",(*allItems)[i].pubDate);
		printf("%s\n",(*allItems)[i].agency);
	}
	printf("\n");

}

static void fillItems(struct item *items){
	int i;
	for(i=0;i<newsItems ->size;i++){
		char *text = ((char **)newsItems->items)[i];//newsItems.items[i];

                char *p = getContent("<pubDate>", "</pubDate>", text);
                if(p==NULL) // some CNN titles are missing pubDate
                        (items[i].pubDate)[0] = '\0';
                else{
                        strcpy(items[i].pubDate, p);
                        free(p); //p was created with malloc
                }

                p = getContent("<title>", "</title>", text);
                if(p==NULL)
                        (items[i].title)[0] = '\0';
                else{
			getTitle(p);
                        items[i].title = p;
                }

		strcpy(items[i].agency, newsItems ->agency);
	}
}

int refreshFeed(struct newsAgency newsAgency)
{
        int k = 0;
        if(newsItems == NULL){
                newsItems = malloc(sizeof(struct news));
                newsItems -> items = malloc(MAX_TITLES * sizeof(char*));
        }
        else if(newsItems -> items != NULL){ //free all the allocated memory
                char **items = (char **)(newsItems -> items);
                for(k=0;k<newsItems -> size; k++){
                        free(items[k]);
                }
        }

    	FILE *fptr;

	/*  open for writing */
        fptr = fopen("feed.xml", "w");
	if (fptr == NULL){
		printf("%s:Could not open file feed.xml for writing \n", newsAgency.url);
		return 1;
	}

	download_feed(fptr, newsAgency.url);
	fclose(fptr);

        int fileSize = fsize("feed.xml");
        if (fileSize == -1){
                printf("%s:Could not stat size of feed.xml\n", "parseXml");
                return 1;
        }

	/*  open for reading */
	fptr = fopen("feed.xml", "r");
	if (fptr == NULL){
		printf("Could not open file feed.xml for reading \n");
		return 1;
	}

        char **news = (char **)newsItems -> items;
        int nlines = 0;

        int state=OUT,i=0,j=0,pos=0;
        char a;
	char buffer[BUFFER_SIZE];
        char identifier[12];
        memset(identifier, 0, 12);

        do { //this do loop will capture all the texts in between item tags
                a = fgetc(fptr);
                for(k=1;k<11;k++){ //shift left
                        identifier[k-1] = identifier[k];
                }
                identifier[k-1]=a; // add new char at the right end
                if(state==OUT){
                        if(strstr(identifier, "<item>") != NULL){
                                state=IN;
				pos=0;
                        }
                }
                else if(state ==IN){
			if(pos<BUFFER_SIZE-1){
				buffer[pos++] = a;
			}
                        if(strstr(identifier, "</item>") != NULL){
                                state=OUT;
				buffer[pos] = '\0';
				cleanItem(buffer);
				cleanHtml(buffer);

				if(nlines<MAX_TITLES && countTitleWords(buffer)){ //
					news[nlines++] = strdup(buffer);
				}
                        }
                }

        } while (a != EOF && ++j<fileSize);// some files don't have EOF

        newsItems ->size = nlines;
	strcpy (newsItems ->agency, newsAgency.name);
	fclose(fptr);
	return 0;
}

void download_feed(FILE *dst, const char *src){
	CURL *handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, src);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, dst);
	curl_easy_perform(handle);
	curl_easy_cleanup(handle); //without this the program will leak memory and eventually crash...learned it the hard way
}

int fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return (st.st_size + 0);

    return -1;
}

void cleanItem(char *buffer){
	char *p = strstr(buffer, "<title>");
	if(!p) return;
	strcpy(buffer, p); //remove all the junk before <title>
	char *p1 = strstr(buffer, "<pubDate>");
	p = strstr(buffer, "</title>");
	if(!p) return;

	if(p1){
		strcpy(p + strlen("</title>"), p1);
		p1 = strstr(buffer, "</pubDate>");
		if(p1)
			*(p1 + strlen("</pubDate>")) = '\0';
	}else{
		*(p + strlen("</title>")) = '\0';
	}
}

void getTitle(char *line){
        static char *suffix = "</TITLE>";
	static char *cdata = "<![CDATA[";
	static char *watch = "WATCH:";
	static char buff[BUFFER_SIZE];

	int i =0;
	while(line[i] && i<BUFFER_SIZE){
		buff[i] = toupper(line[i]);
		i++;
	}
	buff[i] = '\0';//terminate string

        char *p1 = &buff[0];

        if(strstr(p1, cdata) == p1){ // starts with cdata
		p1 += strlen(cdata);
	}

        if(strstr(p1, watch) == p1){ // starts with watch
		p1 += strlen(watch);
	}

	int k=0;
	while(isspace(p1[k])) k++; //get rid of leading whitespace
	p1 += k;

        if(strstr(p1, "]]>") == p1+strlen(p1)-3){ // ends with ]]>
		p1[strlen(p1)-3] = '\0';
        }

	strcpy(line, p1);
}

// returns true if number of words in title > 4
int countTitleWords(char *str)
{
    int state = OUT;
    int wc = 0;  // word count
    char *p = strstr(str, "</title>");
    if(!p) return 0;
    // Scan all characters one by one
    while (str != p)
    {
        // If next character is a separator, set the
        // state as OUT
        if (*str == ' ' || *str == '\n' || *str == '\t')
            state = OUT;

        else if (state == OUT)
        {
            state = IN;
            ++wc;
        }

        ++str;
    }
    return wc > 4 ? 1 : 0;
}

void cleanHtml(char * const line) { // replaces some html codes
	int i;
	char *p;
	static int CODES_LEN = sizeof(codes)/sizeof(codes[0]);
	if(!strstr(line, "&#")) return;
	for(i=0;i<CODES_LEN;i++){
		while((p = strstr(line, codes[i].hcode))){
			*p = codes[i].value;
			*(p+1) = '\0';
			strcat(line, p+strlen(codes[i].hcode));
		}
	}
}

char *getContent(char *starttag, char *endtag, char *text){
        if(!text) return NULL;

        char *p1 = strstr(text, starttag);
        char *p2 = strstr(text, endtag);

        if(!p1 || !p2 || p1>p2) return NULL;

        size_t size = p2 - p1 - strlen(starttag);
        char *out = malloc((size+1) * sizeof(char));

        strncpy(out, p1+strlen(starttag), size);
        out[size] = '\0';
        return out;
}

// utility compare function to be used in qsort
int compare_pubDates(const void* a, const void* b) {
        struct item *itemA = (struct item *)a;
        struct item *itemB = (struct item *)b;
	struct tm tmA,tmB;

	memset(&tmA, 0, sizeof(struct tm));
	memset(&tmB, 0, sizeof(struct tm));

	if(!(itemA -> pubDate)[0]) return 1;
	if(!(itemB -> pubDate)[0]) return -1;

	strptime(itemA -> pubDate,"%a, %d %b %Y %H:%M:%S %Z", &tmA);
	strptime(itemB -> pubDate,"%a, %d %b %Y %H:%M:%S %Z", &tmB);

	if(tmA.tm_year > tmB.tm_year) return -1;
	else if (tmA.tm_year < tmB.tm_year) return 1;
	else if (tmA.tm_mon > tmB.tm_mon) return -1;
	else if (tmA.tm_mon < tmB.tm_mon) return 1;
	else if (tmA.tm_mday > tmB.tm_mday) return -1;
	else if (tmA.tm_mday < tmB.tm_mday) return 1;
	else if (tmA.tm_hour > tmB.tm_hour) return -1;
	else if (tmA.tm_hour < tmB.tm_hour) return 1;
	else if (tmA.tm_min > tmB.tm_min) return -1;
	else if (tmA.tm_min < tmB.tm_min) return 1;
	else if (tmA.tm_sec > tmB.tm_sec) return -1;
	else if (tmA.tm_sec < tmB.tm_sec) return 1;
	else return 0;
}

void cleanRssDateString(char *rssDateString){
	if(!rssDateString[0]) return;
	struct tm tmA;
	memset(&tmA, 0, sizeof(struct tm));
	strptime(rssDateString,"%a, %d %b %Y %H:%M:%S %Z", &tmA);

	char *p = rssDateString;
	p += strlen(p)-5;
	if(*p == '-' || *p == '+'){
		int diff = atoi(p)/100;
		if(diff){
			tmA.tm_hour -= diff;
			if(tmA.tm_hour < 0) tmA.tm_hour += 24;
			strftime(rssDateString, 50, "%a, %d %b %Y %H:%M:%S GMT", &tmA);
               }
       }
}

