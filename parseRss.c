/*
 * C function that gets rss feeds from various news agencies and extracts the titles
 * Uses libcurl open source library to download feed.xml
 * The function accounts that the file downloaded is not necessarily a xml file and may not be "proper"
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
#define  BUFFER_DELTA 100

struct news {
  char agency[10];
  void *items;
  int size;
};

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
static int countWords(char *str);
static void getTitle(char *line);
static int fsize(const char *filename);
static void cleanItem(char *line);
static void cleanHtml(char * const line);  //line start is const
static void fillItems(struct item **items, struct news *newsItems);
static char *getContent(char *starttag, char *endtag, char *text);
static int refreshFeed(struct newsAgency newsAgency, struct news *newsItems);
static int compare_pubDates(const void* a, const void* b);
static void cleanRssDateString(char *rssDateString);


//this function gets exported in parseRss.h
void getLatestItems(int *size, struct item **allItemss){
        int i;
	int currentAllItemsUsed = 0;
	static int currentAllItemsSize = 0;
	static struct news newsItems;
	static struct item *allItems = NULL;

	if(*allItemss){
		for(i=0;i< *size;i++){
			free((*allItemss)[i].title); // free all the previous titles which were created by strdup
		}
	}

        for(i=0;i<NUM_SITES;i++){
                refreshFeed(politics[i], &newsItems);
                if(newsItems.items == NULL) continue;
                struct item *items;
		fillItems(&items, &newsItems);

                if(allItems == NULL){
                        allItems =  items;
			currentAllItemsUsed = currentAllItemsSize = newsItems.size;
                }else{
			if(newsItems.size > (currentAllItemsSize - currentAllItemsUsed)){
	                        allItems =  realloc(allItems, (currentAllItemsSize + BUFFER_DELTA) * sizeof(struct item));
                	        currentAllItemsSize += BUFFER_DELTA;
			}
       	                memcpy (allItems + currentAllItemsUsed, items, newsItems.size * sizeof(struct item));
			currentAllItemsUsed += newsItems.size;
                        free(items); // After previous memcpy it can be freed
                }
        }

	for(i=0;i<currentAllItemsSize;i++){
		cleanRssDateString(allItems[i].pubDate); // attempt to normalize all dates to GMT/UTC time
	}

	qsort(allItems, currentAllItemsSize, sizeof(struct item), compare_pubDates); //sort by pubDate descending

	for(i=0;i<20;i++){
		printf("%s::",allItems[i].title);
		printf("%s::",allItems[i].pubDate);
		printf("%s\n",allItems[i].agency);
	}
	printf("\n");
	*allItemss = allItems;
	*size = currentAllItemsUsed;
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

int refreshFeed(struct newsAgency newsAgency, struct news *newsItems)
{
        int k = 0;
        if(newsItems -> items != NULL){ //free all the allocated memory
                char **itemss = (char **)(newsItems -> items);
                for(k=0;k<newsItems -> size; k++){
			free(itemss[k]);
                }
                free(newsItems -> items);
		newsItems -> items = NULL;
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

        char **news = malloc(MAX_TITLES * sizeof(char*)); // MAX_TITLES titles of BUFFER_SIZE chars each
        int nlines = 0;

        int state=OUT,i=0,j=0,pos=0;
        char a;
	char buffer[BUFFER_SIZE];
        char identifier[12];
        memset(identifier, 0, 12);

        do { //this do loop will capture all the texts in between title tags
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
				if(countWords(buffer)>4 && nlines<MAX_TITLES){ // skip trivial titles
					cleanHtml(buffer);
			                news[nlines++] = strdup(buffer);
				}
                        }
                }

        } while (a != EOF && ++j<fileSize);// some files don't have EOF

        newsItems -> items = news;
        newsItems -> size = nlines;
	strcpy (newsItems -> agency, newsAgency.name);
	fclose(fptr);
	return 0;
}

void download_feed(FILE *dst, const char *src){
	CURL *handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, src);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, dst);
	curl_easy_perform(handle);
}

int fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return (st.st_size + 0);

    return -1;
}

void cleanItem(char *line){
	static char *suffix = "</item>";
	line[strlen(line)-strlen(suffix)] = '\0'; //remove suffix
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

// returns number of words in str
int countWords(char *str)
{
    int state = OUT;
    int wc = 0;  // word count

    // Scan all characters one by one
    while (*str)
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
    return wc;
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

static void fillItems(struct item **itemss, struct news *newsItems){
	struct item *items = malloc(newsItems->size * sizeof(struct item));
	int i;
	for(i=0;i<newsItems->size;i++){
		char *text = ((char **)newsItems->items)[i];
                char *p = getContent("<pubDate>", "</pubDate>", text);
                if(p==NULL) // some CNN titles are missing pubDate
                        (items[i].pubDate)[0] = '\0';
                else{
                        strcpy(items[i].pubDate, p);
                	//free(p); //p was created with strdup
		}
		items[i].title = getContent("<title>", "</title>", text);
		getTitle(items[i].title);
		strcpy(items[i].agency, newsItems->agency);
	}
	*itemss = items;
}

char *getContent(char *starttag, char *endtag, char *text){

	if(!text) return NULL;

	char *p1 = strstr(text, starttag);
	char *p2 = strstr(text, endtag);

	if(!p1 || !p2 || p1>p2) return NULL;

	int size = p2 - p1 - strlen(starttag);
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
