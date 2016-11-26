#ifndef PARSERSS_H_   /* Include guard */
#define PARSERSS_H_

#define NUM_TITLES 20

struct item{
  char title[512];
  char agency[10];
  char pubDate[50];
};

void getLatestItems(struct item **allItemss);
#endif // PARSERSS_H_
