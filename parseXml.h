#ifndef PARSEXML_H_   /* Include guard */
#define PARSEXML_H_


struct news {
  void *titles;
  int size;
};

int refreshFeed(char *url, struct news *newsTitles);



#endif // LCDSHAPES_H_
