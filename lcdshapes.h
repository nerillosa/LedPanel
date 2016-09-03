#ifndef LCDSHAPES_H_   /* Include guard */
#define LCDSHAPES_H_

#include <bcm2835.h>
#include <sys/time.h>

#define LAT RPI_GPIO_P1_07
#define CLK RPI_GPIO_P1_11
#define OE  RPI_GPIO_P1_12

#define R1  RPI_GPIO_P1_23
#define G1  RPI_GPIO_P1_22
#define B1  RPI_GPIO_P1_26

#define R2  RPI_GPIO_P1_24
#define G2  RPI_GPIO_P1_21
#define B2  RPI_GPIO_P1_19

#define AA  RPI_GPIO_P1_15
#define BB  RPI_GPIO_P1_16
#define CC  RPI_GPIO_P1_18

#define NUMBER_PANELS 2
#define NUMBER_ROWS 16
#define NUMBER_COLUMNS_PER_PANEL 32

extern int PANEL_SIZE;
extern int TOTAL_NUMBER_COLUMNS;
extern uint8_t letters[][7];

typedef enum {false=0,true} bool;
typedef enum {blue=1,green,cyan,red,magenta,yellow,white} color;

typedef struct {
  uint8_t x, y;
  color c;
} Point;

int  gpio_init(void);
void toggleClock(void);
void displayRowInit();
void updateRows(uint8_t *displayArray);
static void displayRowEnd(void);

void drawPixel(int x, int y, color c, uint8_t *display);
void drawPoint(Point point, uint8_t *display);
void drawHorizontalLine(int x, int y, int width, color c, uint8_t *display);
void drawVerticalLine(int x, int y, int height, color c, uint8_t *display);
void drawLetter( uint8_t letter, int x, int y, color c, uint8_t *display);
void drawSlantLine (int x, int y, bool isPositiveAngle, int length, color c, uint8_t *display);
void drawRectangle(int x, int y, int width, int height, color c, uint8_t *display);
void drawCircle(int x, int y, int diameter, color c,  uint8_t *display);
void drawSmileyFace(int x, int y, int diameter, uint8_t *display);

long int getTimeDiff(struct timeval a, struct timeval b);
void timevalCopy(struct timeval *dest, struct timeval *source);

#endif // LCDSHAPES_H_
