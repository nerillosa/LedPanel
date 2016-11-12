/*
 * C function that gets rss feed from Fox and extracts the titles
 * There might be easier ways of doing this but this program
 * creates two files: feed.xml and feed.txt
 * Uses libcurl open source library to download the feed.
 * The xml file is the direct feed from the host's website.
 * The text file contains the extracted titles, one on each line.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "parseXml.h"

static void download_feed(FILE *dst, const char *src);
static int peekFile(FILE *dst);
static void getTitle(char *line);

int refreshFeed(char *url)
{
    	FILE *fptr, *tptr;
	/*  open for writing */
        fptr = fopen("feed.xml", "w");
	if (fptr == NULL){
		printf("%s:Could not open file feed.xml for writing \n", url);
		return 1;
	}


	download_feed(fptr, url);
	fclose(fptr);

	/*  open for reading */
	fptr = fopen("feed.xml", "r");
	if (fptr == NULL){
		printf("Could not open file feed.xml for reading \n");
		return 1;
	}

	if(peekFile(fptr) == -1){
                printf("file is not an xml file: %s\n", url);
		fclose(fptr);
		return 1;
	}

	tptr = fopen("feed.txt", "w");

	if (tptr == NULL){
		printf("Could not open file feed.txt for writing \n");
		return 1;
	}

        char line[1024];
	int j = 0;
        while (fgets(line, sizeof(line), fptr)) {
		int i = 0;
                while(line[i]){
	                line[i] = toupper(line[i]);
        	        i++;
              	}
	        getTitle(line);
        	if(strlen(line)){
                	if(++j>2) fprintf(tptr, "%s\n", line); // skip the first two trivial title
		}
	}

	fclose(fptr);
	fclose(tptr);
	return 0;
}

void download_feed(FILE *dst, const char *src){
	CURL *handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, src);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, dst);
	curl_easy_perform(handle);
}

int peekFile(FILE *dst){ // check if its an xml file starting with <?
	int c,d;
	c = fgetc(dst);
	if(c != '<') return -1;
	d = fgetc(dst);
	if(d != '?') return -1;

	ungetc(d, dst);
	ungetc(c, dst);
	return 0;
}

void getTitle(char *line){
        static char *prefix = "<TITLE>";
        static char *suffix = "</TITLE>";
	static char *cdata = "<![CDATA[";

        char *p1 = strstr(line, prefix);
        char *p2 = strstr(line, suffix);
        int i;
        if(p1) p1 += strlen(prefix) + (strstr(line, cdata)? strlen(cdata) : 0);
        if(p1 && p2 && p2>p1){
                for(i=0;i<(p2-p1);i++){
                   line[i] = *(p1+i);
                }
                line[i] = 0;
        }else{
                line[0] = 0;
        }
}

