// butterflies.c
// Running this program shows multicolor butterflies fluttering around.
// The butterflies never leave the view port and never interfere with each other.
// After installing bcm2835, you can build and run this with:
// gcc -o butterflies -l rt butterflies.c lcdshapes.c -l bcm2835
// sudo ./butterflies

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"
#include "lcdfonts.h"

#define MOVE_INTERVAL  150  // time in milliseconds between each move
#define POINTS_PER_BUTTERFLY 3
#define NUM_WING_STATES 3
#define NUM_BUTTERFLIES 10
#define BUTTERFLY_LENGTH 4

static struct butterfly {
   uint8_t pointsOffset;
   color headColor;
   color wingColor;
   short x;
   short y;
} *butterflies = NULL;

void paintCanvas(uint8_t *canvas);
void drawButterfly (struct butterfly bfly, uint8_t *canvas);
void initButterflies();
bool intersectsOther(struct butterfly bfly, int index);
bool butterfliesIntersect(struct butterfly bflyA, struct butterfly bflyB);
void randomizeArray(int* arr, int arr_length, int limit);

struct timeval saved, now;

// Three wing states, 3 points each.
// All three wing states referenced from point 0,0
Point points[] = {{0,0},{2,0},{1,1},{0,1},{1,1},{2,1},{1,1},{0,2},{2,2}};
int len = sizeof(points)/sizeof(Point);

int main(int argc, char **argv)
{
	if (gpio_init()) return 1;
	uint8_t *canvas = (uint8_t *)calloc(PANEL_SIZE, sizeof(uint8_t));
	gettimeofday(&saved, NULL); //start time
	srand(time(NULL));
	while (true) //infinite loop
	{
		displayRowInit();
		paintCanvas(canvas);
		updateRows(canvas);
	}
}

void paintCanvas(uint8_t *canvas){

	if(butterflies == NULL){ //one time initialization
		initButterflies();
	}
 	gettimeofday(&now, NULL);

        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		int i, xoff,yoff;

		for(i=0;i<NUM_BUTTERFLIES;i++){
			if(rand()%2 == 0){
				xoff = rand()%2 ? 1 : -1;
                                butterflies[i].x += xoff;
                                if(butterflies[i].x == 1 || butterflies[i].x == 62 || intersectsOther(butterflies[i], i))
                                	butterflies[i].x -= xoff; // revert if conflict with panel border or other butterfly

                                yoff = rand()%2 ? 1 : -1;
                                butterflies[i].y += yoff;
                                if(butterflies[i].y == 1 || butterflies[i].y == 14 || intersectsOther(butterflies[i], i))
					butterflies[i].y -= yoff; // revert if conflict with panel border or other butterfly
			}
                        drawButterfly (butterflies[i], canvas);

			if(++butterflies[i].pointsOffset == NUM_WING_STATES){
				butterflies[i].pointsOffset = 0;
			}
		}
	}
}

void initButterflies(){
	// 45 possible starting points not intersecting with each other
	// created with pixels.html
	Point points[] = {{4,2},{8,2},{12,2},{16,2},{20,2},{24,2},{28,2},{32,2},{36,2},{40,2},{44,2},{48,2},{52,2},{56,2},{60,2},{4,6},{8,6},{12,6},{16,6},{20,6},{24,6},{28,6},{32,6},{36,6},{40,6},{44,6},{48,6},{52,6},{56,6},{60,6},{4,10},{8,10},{12,10},{16,10},{20,10},{24,10},{28,10},{32,10},{36,10},{40,10},{44,10},{48,10},{52,10},{56,10},{60,10}};
	int pindices[NUM_BUTTERFLIES];

	// fill array with non duplicate random numbers from 0-45
	randomizeArray(pindices, NUM_BUTTERFLIES, 45);

	butterflies = (struct butterfly *) malloc (NUM_BUTTERFLIES * sizeof(struct butterfly));
	int i,j;

	for(i=0;i<NUM_BUTTERFLIES;i++){
		butterflies[i].pointsOffset = 0;
		butterflies[i].headColor = rand()%7 + 1;
		while(true){
			j = rand()%7 + 1;
			if(j != butterflies[i].headColor){
				butterflies[i].wingColor = j;
				break;
			}
		}
		butterflies[i].x = points[pindices[i]].x;
		butterflies[i].y = points[pindices[i]].y;
	}

}

void drawButterfly (struct butterfly bfly, uint8_t *canvas){
	Point *point = points + bfly.pointsOffset * POINTS_PER_BUTTERFLY;
	int i;
	color clr;
	for(i=0;i<POINTS_PER_BUTTERFLY;i++){
		if((point + i) ->x ==1 && (point + i) ->y == 1){ // {1,1} the head
			clr = bfly.headColor;
		}else{
			clr = bfly.wingColor;
		}
		drawPixel((point + i) ->x + bfly.x, (point + i) ->y + bfly.y, clr, canvas);
	}
}

bool intersectsOther(struct butterfly bfly, int index){
        int i;
        for(i=0;i<NUM_BUTTERFLIES;i++){
                if(i==index){ // don't compare with yourself
                        continue;
                }
                if(butterfliesIntersect(bfly, butterflies[i]))
                        return true;
        }
        return false;
}

bool butterfliesIntersect(struct butterfly bflyA, struct butterfly bflyB){
	return (bflyA.x+BUTTERFLY_LENGTH >= bflyB.x && bflyB.x+BUTTERFLY_LENGTH >= bflyA.x && 
		bflyA.y+BUTTERFLY_LENGTH >= bflyB.y && bflyB.y+BUTTERFLY_LENGTH >= bflyA.y);
}

void randomizeArray(int* arr, int arr_length, int limit){
        int i,j,k;
        bool done = true;
        //Fill it. Don't worry about duplicates...yet
        for(k=0;k<arr_length;k++){
                *(arr+k) = rand()%limit;
        }
        while(true){// check for duplicates and correct
                for (i = 0; i < arr_length - 1; i++) {
                        for (j = i + 1; j < arr_length; j++) {
                                if (arr[i] == arr[j]) {
                                        arr[i] = rand()%limit;
                                        done=false;
                                        break;
                                }
                        }
                        if(!done){
                                break;
                        }
                }
                if(!done)
                        done=true;
                else
                        break;
        }
}

