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
#include <ctype.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include "parseXml.h"

static void download_feed(FILE *dst, const char *src);
//static int peekFile(FILE *dst);
static void getTitle(char *line);
int fsize(const char *filename);

int refreshFeed(char *url)
{
    	FILE *fptr, *tptr;
        char identifier[12];
        memset(identifier, 0, 12);
        char a;
        int state=0;


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

	tptr = fopen("feed.txt", "w");

	if (tptr == NULL){
		printf("Could not open file feed.txt for writing \n");
		return 1;
	}

        int i=0,j=0;
	int pos = 0;
	char buffer[512];
        do { //this do loop will find all the text in between title tags
                a = fgetc(fptr);
                int k;
                for(k=1;k<11;k++){
                        identifier[k-1] = identifier[k];
                }
                identifier[k-1]=a;
                if(state==0){
                        if(strstr(identifier, "<title>") != NULL){
                                state=1;
				pos=0;
                        }
                }
                else if(state ==1){
                        buffer[pos++] = a;
                        if(strstr(identifier, "</title>") != NULL){
                                state=0;
				buffer[pos] = '\0';
				if(++i>2){ // skip the first two titles
					getTitle(buffer);
					fprintf(tptr, "%s\n", buffer);
				}
                        }
                }

        } while (a != EOF && ++j<fileSize);// some files don't have EOF

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

int fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return (st.st_size + 0);

    return -1;
}


void getTitle(char *line){
        static char *suffix = "</TITLE>";
	static char *cdata = "<![CDATA[";
	int i =0;
	while(line[i]){
		line[i] = toupper(line[i]);
		i++;
	}
        char *p1 = &line[0];
        char *p2 = strstr(line, suffix);

        p1 += (strstr(line, cdata)? strlen(cdata) : 0);
        if(p2 && p2>p1){
                for(i=0;i<(p2-p1);i++){
                   line[i] = *(p1+i);
                }
                line[i] = 0;
        }else{
                line[0] = 0;
        }

	int k=0;
	char *bandido = NULL;

	if(strstr(line, "WATCH:") == &line[0]){ // starts with WATCH:
		bandido = strdup(&line[0]+6);
		while(isspace(bandido[k])) k++;
		strcpy(line, bandido+k);
		free(bandido);
		return;
	}

	while(isspace(line[k])) k++; //get rid of leading whitespace

	if(k){
		bandido = strdup(&line[0]+k);
		strcpy(line, bandido);
		free(bandido);
	}


}

