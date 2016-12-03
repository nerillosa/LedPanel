/*
 * C function that gets the latest rss feeds from various news agencies and extracts the titles
 * Uses libcurl open source library to download feed.xml
 * The function accounts that the file downloaded is not necessarily a xml file and may not be "proper"
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "parseRss.h"

#define OUT 0
#define IN  1
#define MAX_TITLES 100
#define BUFFER_SIZE 8192
#define ITEM_SIZE 200

struct news {
  char agency[10];
  char items[ITEM_SIZE][512];
  int size;
} newsItems;

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
static void getContent(int dest_size, char *dest, char *starttag, char *endtag, char *text);
static int refreshFeed(struct newsAgency newsAgency);
static int compare_pubDates(const void* a, const void* b);
static void cleanRssDateString(char *rssDateString);
static void getJsonFromItems(int size, struct item *items, char *json);
static void sendPostNews(char *jsonStringValue);
static void cleanJson(char *json);
static void cleanUrl(char *url);
static void cleanTitle(int size, char *buff);
static int calcDays(int month, int year);

static struct item itemArray[ITEM_SIZE];

//this function gets exported in parseRss.h
void getLatestItems(struct item **allItemss){
        int i,j;
	int currentItemsCount = 0;

        for(i=0;i<NUM_SITES;i++){
                refreshFeed(politics[i]);
		fillItems(&itemArray[0] + currentItemsCount);
		currentItemsCount += newsItems.size;
        }

	printf("currentItemsCount:%d\n",currentItemsCount);

	for(i=0;i<currentItemsCount;i++){
		cleanRssDateString(itemArray[i].pubDate); // attempt to normalize all dates to GMT/UTC time
	}

	qsort(itemArray, currentItemsCount, sizeof(struct item), compare_pubDates); //sort by pubDate descending

	for(i=0;i<NUM_TITLES;i++){
		printf("%s::",itemArray[i].title);
		printf("%s::",itemArray[i].pubDate);
//		printf("%s::",itemArray[i].url);
		printf("%s\n",itemArray[i].agency);
	}
	printf("\n");

	*allItemss = itemArray;
	wait(NULL); // kill previous child zombie

	pid_t child = fork(); // fork a child process to post the data to the website
	if(child == 0){ //this is the child process
		char json[BUFFER_SIZE];
		getJsonFromItems(NUM_TITLES, itemArray, json);
		cleanJson(json);
		sendPostNews(json);
		_exit(0);
  	}
}

static void fillItems(struct item *items){
	int i;
	for(i=0;i<newsItems.size;i++){
		char *text = newsItems.items[i];
                getContent(50, items[i].pubDate, "<pubDate>", "</pubDate>", text);
		getContent(512, items[i].title, "<title>", "</title>", text);
		getContent(256, items[i].url, "<link>", "</link>", text);
		cleanUrl(items[i].url);
		getTitle(items[i].title);
		strcpy(items[i].agency, newsItems.agency);
	}
}

int refreshFeed(struct newsAgency newsAgency)
{
        int k = 0;
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

        //char **news = newsItems.items;
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

				if(nlines<MAX_TITLES && countTitleWords(buffer)){
			                strncpy(newsItems.items[nlines], buffer, 512);
			                newsItems.items[nlines++][511] = '\0';
//					printf("%s\n\n", buffer);
				}
                        }
                }

        } while (a != EOF && ++j<fileSize);// some files don't have EOF

        newsItems.size = nlines;
	strcpy (newsItems.agency, newsAgency.name);
	fclose(fptr);
	return 0;
}

void download_feed(FILE *dst, const char *src){
	CURL *handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, src);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, dst);
	curl_easy_perform(handle);
	curl_easy_cleanup(handle); //without this program will leak memory and eventually crash...learned it the hard way
}

int fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return (st.st_size + 0);

    return -1;
}

void cleanItem(char *buffer){
	char temp[1024];
	memset(temp, 0, 1024);

	char *p = strstr(buffer, "<title>");
	char *p1 = strstr(buffer, "</title>");

	if(!p || !p1) return;
	strncpy(temp, p, p1-p + strlen("</title>"));

	p = strstr(buffer, "<link>");
	p1 = strstr(buffer, "</link>");

	if(!p || !p1) return;
	strncat(temp, p, p1-p + strlen("</link>"));

	p = strstr(buffer, "<pubDate>");
	p1 = strstr(buffer, "</pubDate>");

	if(p && p1){
		strncat(temp, p, p1-p + strlen("</pubDate>"));
	}

	strcpy(buffer, temp);
//	printf("%d:%s\n",strlen(buffer),buffer);
}

void getTitle(char *line){
        static char *suffix = "</TITLE>";
	static char *cdata = "<![CDATA[";
	static char *watch = "WATCH:";
	static char buff[BUFFER_SIZE];

	int i =0;
	while(line[i] && i<BUFFER_SIZE-1){
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

	int size = BUFFER_SIZE - (p1 - buff);
	cleanTitle(size, p1);

	strcpy(line, p1);
}

//Removes 3 consecutive chars with MSB set to 1 and replaces them with a '\''
void cleanTitle(int size, char *buff){
        int i;
        for(i=0;i<size;i++){
                if(buff[i]>>7){
                  strncpy( buff+i, buff+i+2, size-i-2);
                  buff[i] = '\'';
                  cleanTitle(size, buff); //recursive in case there are more than one ocurrences of weird quotes
                  break;
                }
        }
}


void cleanUrl(char *url){
	static char *cdata = "<![CDATA[";

        if(strstr(url, cdata) != &url[0]) return;
        int cdataSize = strlen(cdata);
        int last = strlen(url) - cdataSize - 3;
        memcpy(url, &url[cdataSize], strlen(url) - cdataSize);
        url[last] = '\0';
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

void getContent(int dest_size, char *dest, char *starttag, char *endtag, char *text){

	if(!text){
		dest[0] = '\0';
		return;
	}

	char *p1 = strstr(text, starttag);
	char *p2 = strstr(text, endtag);

	if(!p1 || !p2 || p1>p2){
		dest[0] = '\0';
		return;
	}

	size_t size = p2 - p1 - strlen(starttag);

	strncpy(dest, p1+strlen(starttag), dest_size);
	dest[size] = '\0';

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

        int i=0,k=0;
        while(rssDateString[i]){
                if(rssDateString[i++] == ':') k++;
        }
        if(!k) return;
        if(k==1){ // its missing the seconds count
                int sz = strlen(rssDateString);
                for(i=0;i<5;i++){
                        rssDateString[sz + 3 -i] = rssDateString[sz-i];
                }
                rssDateString[sz-4] = ':';
                rssDateString[sz-3] = '0';
                rssDateString[sz-2] = '0';
        }


	struct tm tmA;
	memset(&tmA, 0, sizeof(struct tm));
	strptime(rssDateString,"%a, %d %b %Y %H:%M:%S %Z", &tmA);

	char *p = rssDateString;
	p += strlen(p)-5;
	if(*p == '-' || *p == '+'){
		int diff = atoi(p)/100;
		if(diff){
			tmA.tm_hour -= diff;
			if(tmA.tm_hour < 0) {
				tmA.tm_hour += 24;
				if(tmA.tm_mday > 1){
					tmA.tm_mday -= 1;
				}else{
					tmA.tm_mon -= 1;
					if(tmA.tm_mon < 0){ // its January 1st!
						tmA.tm_mon = 11;
						tmA.tm_year -= 1;
					}else
						tmA.tm_mday = calcDays(tmA.tm_mon, 1900 + tmA.tm_year);
				}
			}
			strftime(rssDateString, 50, "%a, %d %b %Y %H:%M:%S GMT", &tmA);
               }
       }
}

int calcDays(int month, int year)// calculates number of days in a given month
{
	int Days;
	if (month == 3 || month == 5 || month == 8 || month == 10) Days = 30;
	else if (month == 1) {
		int isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    		if (isLeapYear) Days = 29;
    		else Days = 28;
	}
	else Days = 31;
	return Days;
}



void getJsonFromItems(int size, struct item *items, char *json){
        strcpy(json, "value=[");
        int i;
        for(i=0;i<size;i++){
                strcat(json, "{\"title\":\"");
                strcat(json, items[i].title);
                strcat(json, "\",\"");
                strcat(json, "url\":\"");
                strcat(json, items[i].url);
                strcat(json, "\",\"");
                strcat(json, "agency\":\"");
                strcat(json, items[i].agency);
                strcat(json, "\",\"");
                strcat(json, "pubdate\":\"");
                strcat(json, items[i].pubDate);
                strcat(json, "\"},");
        }
                json[strlen(json)-1]= ']';
}

void sendPostNews(char *jsonStringValue){
	CURL *curl;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_URL, "http://llosa.org/neri/updateNews.php");
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStringValue);
	curl_easy_perform(curl);
	curl_easy_cleanup(curl);
}

void cleanJson(char *json){//urlencodes all ampersands as %26
        char *p;
        int i;
        while((p = strstr(json, "&"))){
                size_t sz = strlen(json);
                size_t rr = p - &json[0];
                for(i=0;i<sz-rr;i++){
                        json[sz+1-i]=json[sz-1-i];
                }
                *p = '%';
                *(p+1) = '2';
                *(p+2) = '6';
                json[sz+2] = '\0';
        }

}




