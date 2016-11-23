#ifndef PARSERSS_H_   /* Include guard */
#define PARSERSS_H_

struct item{
  char *title;
  char agency[10];
  char pubDate[50];
};

void getLatestItems(int *size, struct item **allItemss);

#endif // PARSERSS_H_
