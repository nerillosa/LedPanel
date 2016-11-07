/*
 * C program that gets rss feed from Fox and extracts the titles
 * There might be easier ways of doing this but this program
 * creates two files: feed.xml and feed.txt
 * The xml file is the direct feed from the host's website.
 * The text file is the extracted titles, one on each line.
 * To run this program you will need to install libCurl and Mini-XML version 2.9
 * To make it work I had to copy (once installed) the mini-XML shared library (.so) from
 * /usr/local/lib to /usr/lib because the compiler was not finding it.
 *
 * To compile: gcc -o parseXml parseXml.c -lmxml -lcurl -pthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <mxml.h>

void download_feed(FILE *dst, const char *src);
int peekFile(FILE *dst);

int main(int argc, char **argv)
{
	char *url = "http://feeds.foxnews.com/foxnews/world?format=xml";
	if(argc > 1){
		url = argv[1];
	}

    	FILE *fptr, *tptr;
	/*  open for writinging */
        fptr = fopen("feed.xml", "w");
	if (fptr == NULL){
		printf("%s:Could not open file feed.xml for writing \n", argv[0]);
		return 1;
	}


	download_feed(fptr, url);
	fclose(fptr);

	mxml_node_t *tree;

	int whitespace = 0;
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

	tree = mxmlLoadFile(NULL, fptr, MXML_TEXT_CALLBACK);
	mxml_node_t *node, *titleNode;
	node = mxmlFindElement(tree, tree, "item", NULL, NULL, MXML_DESCEND);

	titleNode = mxmlFindElement(node, tree, "title", NULL, NULL, MXML_DESCEND);
	titleNode = mxmlGetFirstChild(titleNode);
	int type = mxmlGetType(titleNode);
        if(type == MXML_TEXT)
		fprintf(tptr, "%s", mxmlGetText(titleNode, &whitespace));
	else{
		fprintf(tptr, "%s", mxmlGetCDATA(titleNode));
	}

	while(1){
		titleNode = mxmlGetNextSibling(titleNode);
		if(titleNode == NULL) break;
		fprintf(tptr, " %s", mxmlGetText(titleNode, &whitespace));
	}

	fprintf(tptr, "\n");

	node = mxmlGetNextSibling(node);

	while(node != NULL){
		titleNode = mxmlFindElement(node, tree, "title", NULL, NULL, MXML_DESCEND);
		titleNode = mxmlGetFirstChild(titleNode);
		if(titleNode != NULL){
			int type = mxmlGetType(titleNode);
       			if(type == MXML_TEXT)
				fprintf(tptr, "%s", mxmlGetText(titleNode, &whitespace));
			else{
				fprintf(tptr, "%s", mxmlGetCDATA(titleNode));
			}
		}
		while(1){
			titleNode = mxmlGetNextSibling(titleNode);
			if(titleNode == NULL) break;
//                        int type = mxmlGetType(titleNode);
//                        if(type == MXML_TEXT)
                                fprintf(tptr, " %s", mxmlGetText(titleNode, &whitespace));
//                        else{
//                                fprintf(tptr, " %s", mxmlGetCDATA(titleNode));
//                        }
		}

		node = mxmlGetNextSibling(node);
		node = mxmlGetNextSibling(node);
		if(node != NULL)
			fprintf(tptr, "\n");
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
