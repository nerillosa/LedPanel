/*
 * C function that gets rss feeds from various news agencies and extracts the titles
 * Uses libcurl open source library to download feed.xml
 * The function accounts that the file downloaded is not necessarily a xml file, may not be "proper"
 * The text file feed.txt created contains the extracted titles, one on each line.
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

static void download_feed(FILE *dst, const char *src);
int countWords(char *str);
static void getTitle(char *line);
int fsize(const char *filename);

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

        int state=OUT,i=0,j=0,pos=0, k;
        char a;
	char buffer[512];
        char identifier[12];
        memset(identifier, 0, 12);

        do { //this do loop will find all the text in between title tags
                a = fgetc(fptr);
                for(k=1;k<11;k++){
                        identifier[k-1] = identifier[k];
                }
                identifier[k-1]=a;
                if(state==OUT){
                        if(strstr(identifier, "<title>") != NULL){
                                state=IN;
				pos=0;
                        }
                }
                else if(state ==IN){
                        buffer[pos++] = a;
                        if(strstr(identifier, "</title>") != NULL){
                                state=OUT;
				buffer[pos] = '\0';
				getTitle(buffer);
				if(countWords(buffer)>5){ // skip trivial titles
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
	static char *watch = "WATCH:";
	int i =0;
	static char buff[512];
	while(line[i] && i<512){
		buff[i] = toupper(line[i]);
		i++;
	}
	buff[i-strlen(suffix)] = '\0'; //remove suffix

        char *p1 = &buff[0];

        if(strstr(p1, cdata) == p1){ // starts with cdata
		p1 += strlen(cdata);
	}

        if(strstr(p1, watch) == p1){ // starts with watch
		p1 += strlen(cdata);
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
