/*
* C function that gets the latest rss feeds from various news agencies and extracts the titles
* Uses libcurl open source library to download feed.xml
* The function accounts that the file downloaded is not necessarily a xml file and may not be "proper"
* Compile with : gcc createNews.c -o createNews -lcurl `mysql_config --cflags --libs`
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <my_global.h>
#include <mysql.h>

#define OUT 0
#define IN  1
#define MAX_TITLES 100
#define BUFFER_SIZE 8192
#define ITEM_SIZE 200
#define NUM_TITLES 20

struct item{
	char title[512];
	char agency[10];
	char pubDate[50];
	char url[256];
};

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
        int type;
        char name[10];
        char *url;
} politics[] =  {
                {1,"FOX NEWS","http://feeds.foxnews.com/foxnews/politics?format=xml"},
                {2,"FOX NEWS","http://feeds.foxnews.com/foxnews/science?format=xml"},
                {4,"FOX NEWS","http://feeds.foxnews.com/foxnews/sports?format=xml"},
                {5,"FOX NEWS","http://feeds.foxnews.com/foxnews/entertainment?format=xml"},
                {7,"FOX NEWS","http://feeds.foxnews.com/foxnews/national?format=xml"},

                {1,"NY TIMES","http://rss.nytimes.com/services/xml/rss/nyt/Politics.xml"},
                {2,"NY TIMES","http://rss.nytimes.com/services/xml/rss/nyt/Science.xml"},
                {3,"NY TIMES","http://rss.nytimes.com/services/xml/rss/nyt/World.xml"},
                {4,"NY TIMES","http://rss.nytimes.com/services/xml/rss/nyt/Sports.xml"},
                {6,"NY TIMES","http://rss.nytimes.com/services/xml/rss/nyt/Health.xml"},

                {1,"CNN","http://rss.cnn.com/rss/cnn_allpolitics.rss"},
                {3,"CNN","http://rss.cnn.com/rss/cnn_world.rss"},
                {5,"CNN","http://rss.cnn.com/rss/cnn_showbiz.rss"},
                {6,"CNN","http://rss.cnn.com/rss/cnn_health.rss"},
                {7,"CNN","http://rss.cnn.com/rss/cnn_us.rss"},

                {1,"WSH POST","http://feeds.washingtonpost.com/rss/rss_powerpost"},
		{2,"WSH POST","http://www.washingtonpost.com/wp-dyn/rss/technology/index.xml"},
                {6,"WSH POST","http://www.washingtonpost.com/wp-dyn/rss/health/index.xml"},
                {7,"WSH POST","http://www.washingtonpost.com/wp-dyn/rss/nation/index.xml"},

                {1,"CNBC","http://www.cnbc.com/id/10000113/device/rss/rss.html"},
                {2,"CNBC","http://www.cnbc.com/id/19854910/device/rss/rss.html"},
                {3,"CNBC","http://www.cnbc.com/id/100727362/device/rss/rss.html"},
                {6,"CNBC","http://www.cnbc.com/id/10000108/device/rss/rss.html"},
                {7,"CNBC","http://www.cnbc.com/id/15837362/device/rss/rss.html"},

                {1,"ABC NEWS","http://feeds.abcnews.com/abcnews/politicsheadlines"},
                {2,"ABC NEWS","http://feeds.abcnews.com/abcnews/technologyheadlines"},
                {3,"ABC NEWS","http://feeds.abcnews.com/abcnews/internationalheadlines"},
		{4,"ABC NEWS","http://feeds.abcnews.com/abcnews/sportsheadlines"},
                {5,"ABC NEWS","http://feeds.abcnews.com/abcnews/entertainmentheadlines"},
                {6,"ABC NEWS","http://feeds.abcnews.com/abcnews/healthheadlines"},
                {7,"ABC NEWS","http://feeds.abcnews.com/abcnews/usheadlines"},

                {1,"REUTERS","http://feeds.reuters.com/Reuters/PoliticsNews"},
                {2,"REUTERS","http://feeds.reuters.com/reuters/scienceNews"},
                {3,"REUTERS","http://feeds.reuters.com/Reuters/worldNews"},
                {4,"REUTERS","http://feeds.reuters.com/reuters/sportsNews"},
                {5,"REUTERS","http://feeds.reuters.com/reuters/entertainment"},
                {6,"REUTERS","http://feeds.reuters.com/reuters/healthNews"},
                {7,"REUTERS","http://feeds.reuters.com/Reuters/domesticNews"},

                {1,"US TODAY","http://rssfeeds.usatoday.com/usatodaycomwashington-topstories&x=1"},
                {4,"US TODAY","http://rssfeeds.usatoday.com/usatodaycomsports-topstories&x=1"},
                {5,"US TODAY","http://rssfeeds.usatoday.com/usatoday-lifetopstories&amp;x=1"},
                {7,"US TODAY","http://rssfeeds.usatoday.com/usatodaycomnation-topstories&x=1"},

                };



int NUM_SITES = sizeof(politics)/sizeof(politics[0]);
static struct item itemArray[ITEM_SIZE];


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

static void getInsertString(struct item *item, char *json, int type);
static void cleanDateString(char *rssDateString);
static void escapeQuotes(char *title);

static void cleanUrl(char *url);
static void cleanTitle(int size, char *buff);
static int calcDays(int month, int year);
static void getLatestItems(int type);
static void downloadFeeds(int type);


int main(int argc, char *argv[]){
	while(1){
		int i;
		for(i=1;i<=7;i++)
			getLatestItems(i);
		sleep(300); // sleep for 5 minutes
	}
}

static void getLatestItems(int type){
	int i,j;
	int currentItemsCount = 0;

	downloadFeeds(type);
	for(i=0;i<NUM_SITES;i++){
		if(politics[i].type != type) continue;
		refreshFeed(politics[i]);
		fillItems(&itemArray[0] + currentItemsCount);
		currentItemsCount += newsItems.size;
	}

	printf("currentItemsCount:%d\n",currentItemsCount);

	for(i=0;i<currentItemsCount;i++){
		cleanRssDateString(itemArray[i].pubDate); // attempt to normalize all dates to MST time
	}

	qsort(itemArray, currentItemsCount, sizeof(struct item), compare_pubDates); //sort by pubDate descending

	for(i=0;i<NUM_TITLES;i++){
		printf("%s::",itemArray[i].title);
		printf("%s::",itemArray[i].pubDate);
		printf("%s\n",itemArray[i].agency);
	}
	printf("\n\n");

	char buff[1024];

	MYSQL *con = mysql_init(NULL);

	if (con == NULL){
		fprintf(stderr, "mysql_init() failed\n");
		return;
	}
	if (mysql_real_connect(con, "localhost", "*******", "*******", "*******", 0, NULL, 0) == NULL){
		fprintf(stderr, "%s\n", mysql_error(con));
		mysql_close(con);
		return;
	}
	for(i=0;i<NUM_TITLES;i++){
		getInsertString(&itemArray[i], buff, type);
		puts(buff);
		if (mysql_query(con, buff)){
			fprintf(stderr, "%s\n", mysql_error(con));
		}
	}

	//Leave at least 80 records for each category
	strcpy(buff, "DELETE FROM news WHERE pubdate < (select pubdate from (select * from news)a where news_type=");
        char beth[5];
        sprintf(beth, "%d", type);
	strcat(buff, beth);
	strcat(buff, " order by pubdate desc limit 79,1) and news_type=");
	strcat(buff, beth);
	if (mysql_query(con, buff)){
		fprintf(stderr, "%s\n", mysql_error(con));
	}

	mysql_close(con);
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

	int fileSize = fsize(newsAgency.name);
	if (fileSize == -1){
		printf("%s:Could not stat size of feed file\n", newsAgency.name);
		return 1;
	}

	/*  open for reading */
	fptr = fopen(newsAgency.name, "r");
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
	escapeQuotes(p1);
	strcpy(line, p1);
}

//Removes 3 consecutive chars with MSB (utf-8) set to 1 and replaces them with a '\''
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
	int diff = 7; // MST = UTC -7
	int delta= 0;
	if(*p == '-' || *p == '+'){
		delta = atoi(p)/100;
	}
	diff += delta;
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

void getInsertString(struct item *item, char *json, int type){
	char beth[5];
	char rssdate[50];
	strcpy(rssdate, item ->pubDate);
	cleanDateString(rssdate);
	sprintf(beth, "%d", type);
	strcpy(json, "REPLACE INTO news (news_type,title,url,pubdate,agency) values (");
	strcat(json, beth);
	strcat(json, ",'");
	strcat(json, item ->title);
	strcat(json, "','");
	strcat(json, item ->url);
	strcat(json, "',STR_TO_DATE('");
	strcat(json, rssdate);
	strcat(json, "','%Y-%m-%d %H:%i:%S'),'");
	strcat(json, item ->agency);
	strcat(json, "')");
}

void cleanDateString(char *rssDateString){
        struct tm tmA;
        memset(&tmA, 0, sizeof(struct tm));
        strptime(rssDateString,"%a, %d %b %Y %H:%M:%S %Z", &tmA);
        strftime(rssDateString, 50, "%Y-%m-%d %H:%M:%S", &tmA);
}

void escapeQuotes(char *title){//add another quote to a quote: ''
        char *p;
        int i;
        if(p = strstr(title, "'")){
                size_t sz = strlen(title);
                size_t rr = p - &title[0];
                for(i=0;i<sz-rr;i++){
                        title[sz+1-i]=title[sz-i];
                }
                *(p+1) = '\'';
                title[sz+1] = '\0';
                if(strlen(p+2))
                        escapeQuotes(p+2);
        }
}

void downloadFeeds(int type){
	int i;
	for(i=0;i<NUM_SITES;i++){
		if (politics[i].type==type && fork() == 0 ){
			FILE *fptr;

			fptr = fopen(politics[i].name, "w");
			if (fptr == NULL){
				printf("%s:Could not open file for writing \n", politics[i].name);
				_exit(1);
			}

			download_feed(fptr, politics[i].url);
			fclose(fptr);
			_exit(0);
		}
	}
	//parent code
	pid_t pid;
	// wait for ALL children to terminate
	do {
		pid = wait(NULL);
	} while (pid != -1);

}


