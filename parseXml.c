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
#include "parseXml.h"

#define OUT 0
#define IN  1
#define MAX_TITLES 100
#define BUFFER_SIZE 512

static void download_feed(FILE *dst, const char *src);
static int countWords(char *str);
static void getTitle(char *line);
static int fsize(const char *filename);

int refreshFeed(char *url, struct news *newsTitles)
{
        int k = 0;
        if(newsTitles -> titles != NULL){ //free all the allocated memory
                char **titless = (char **)(newsTitles -> titles);
                for(k=0;k<newsTitles -> size; k++){
                        free(titless[k]);
                }
                free(newsTitles -> titles);
        }

    	FILE *fptr;

	/*  open for writing */
        fptr = fopen("feed.xml", "w");
	if (fptr == NULL){
		printf("%s:Could not open file feed.xml for writing \n", url);
		return 1;
	}

	download_feed(fptr, url);
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
                        if(strstr(identifier, "<title>") != NULL){
                                state=IN;
				pos=0;
                        }
                }
                else if(state ==IN){
			if(pos<BUFFER_SIZE-1){
				buffer[pos++] = a;
			}
                        if(strstr(identifier, "</title>") != NULL){
                                state=OUT;
				buffer[pos] = '\0';
				getTitle(buffer);
				if(countWords(buffer)>4 && nlines<MAX_TITLES){ // skip trivial titles
			                news[nlines++] = strdup(buffer);
				}
                        }
                }

        } while (a != EOF && ++j<fileSize);// some files don't have EOF

        newsTitles -> titles = news;
        newsTitles -> size = nlines;

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
	buff[i-strlen(suffix)] = '\0'; //remove suffix

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
