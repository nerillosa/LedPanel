// butterflies.c
// Running this program shows multicolor butterflies fluttering around.
// The butterflies will eventually all settle down on a perching point.
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
#define NUM_BUTTERFLIES 5
#define BUTTERFLY_LENGTH 4

static color sclr = blue;

static struct butterfly {
   uint8_t pointsOffset;
   color headColor;
   color wingColor;
   short x;
   short y;
   uint8_t moving;
} *butterflies = NULL;

void paintCanvas(uint8_t *canvas);
void drawButterfly (struct butterfly *bfly, uint8_t *canvas);
void initButterflies();
bool intersectsOther(struct butterfly bfly, int index);
bool butterfliesIntersect(struct butterfly bflyA, struct butterfly bflyB);
void randomizeArray(int* arr, int arr_length, int limit);
void paintSupports(uint8_t *canvas);
void checkMove(struct butterfly *bfly);
void drawDoneMessage(uint8_t *canvas);
void randomizeButterflies();
int  compare_points(const void* a, const void* b);

struct timeval saved, now;

// Three wing states, 3 points each.
// All three wing states referenced from point 0,0
Point points[] = {{0,0},{2,0},{1,1},{0,1},{1,1},{2,1},{1,1},{0,2},{2,2}};

Point perchPoints[] = {{4,13},{10,13},{16,13},{22,13},{28,13},{34,13},{40,13},{46,13},{52,13},{58,13},{3,14},{4,14},{5,14},{9,14},{10,14},{11,14},{15,14},{16,14},{17,14},{21,14},{22,14},{23,14},{27,14},{28,14},{29,14},{33,14},{34,14},{35,14},{39,14},{40,14},{41,14},{45,14},{46,14},{47,14},{51,14},{52,14},{53,14},{57,14},{58,14},{59,14},{2,15},{3,15},{4,15},{5,15},{6,15},{8,15},{9,15},{10,15},{11,15},{12,15},{14,15},{15,15},{16,15},{17,15},{18,15},{20,15},{21,15},{22,15},{23,15},{24,15},{26,15},{27,15},{28,15},{29,15},{30,15},{32,15},{33,15},{34,15},{35,15},{36,15},{38,15},{39,15},{40,15},{41,15},{42,15},{44,15},{45,15},{46,15},{47,15},{48,15},{50,15},{51,15},{52,15},{53,15},{54,15},{56,15},{57,15},{58,15},{59,15},{60,15}};
int slen = sizeof(perchPoints)/ sizeof(Point);


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
	static int state = 0;
	if(butterflies == NULL){ //one time initialization
		initButterflies();
	}
 	gettimeofday(&now, NULL);

        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		paintSupports(canvas);
		int i, xoff,yoff;
		bool allperched = true;
		for(i=0;i<NUM_BUTTERFLIES;i++){
			if(butterflies[i].moving && rand()%2 == 0){
				xoff = rand()%2 ? 1 : -1;
                                butterflies[i].x += xoff;
                                if(butterflies[i].x == 1 || butterflies[i].x == 62 || intersectsOther(butterflies[i], i))
                                	butterflies[i].x -= xoff; // revert if conflict with panel border or other butterfly

                                yoff = rand()%2 ? 1 : -1;
                                butterflies[i].y += yoff;
                                if(butterflies[i].y == 1 || butterflies[i].y == 12 || intersectsOther(butterflies[i], i))
					butterflies[i].y -= yoff; // revert if conflict with panel border or other butterfly
			}
                        drawButterfly (butterflies + i, canvas);

			if(++butterflies[i].pointsOffset == NUM_WING_STATES){
				butterflies[i].pointsOffset = 0;
			}
			if(butterflies[i].moving) allperched = false;
		}
		if(allperched){
			drawDoneMessage(canvas);
			if(++state == 20){ // show the done message for 20 MOVE_INTERVALs (~3 sec)
				state = 0;
				randomizeButterflies(); //re-randomize butterflies
				if(++sclr == 8) // shuffle the perch colors
					sclr = blue;

			}

		}
	}
}

// renders all the perching points and colors them
void paintSupports(uint8_t *canvas){
	int i;
	color clr = sclr;
	for(i=0;i<slen;i++){
		if(i%9 == 0){ // each perch has nine points total
			if(++clr == 8) // seven colors
				clr = blue;
		}
		drawPixel(perchPoints[i].x, perchPoints[i].y, clr, canvas);
	}
}

void initButterflies(){
	//sort the perch points by their x value to color them uniformly
	qsort(perchPoints, slen, sizeof(Point), compare_points);
	butterflies = (struct butterfly *) malloc (NUM_BUTTERFLIES * sizeof(struct butterfly));
	randomizeButterflies();
}

void randomizeButterflies(){
	// 30 possible starting points not intersecting with each other
	// created with pixels.html
	Point ppoints[] = {{4,2},{8,2},{12,2},{16,2},{20,2},{24,2},{28,2},{32,2},{36,2},{40,2},{44,2},{48,2},{52,2},{56,2},{60,2},{4,6},{8,6},{12,6},{16,6},{20,6},{24,6},{28,6},{32,6},{36,6},{40,6},{44,6},{48,6},{52,6},{56,6},{60,6}};
	int pindices[NUM_BUTTERFLIES];

	// fill array with non duplicate random numbers from 0-29
	randomizeArray(pindices, NUM_BUTTERFLIES, 30);

	int i,j;
	for(i=0;i<NUM_BUTTERFLIES;i++){
		butterflies[i].moving = 1;
		butterflies[i].pointsOffset = rand()%NUM_WING_STATES;
		butterflies[i].headColor = rand()%7 + 1;
		while(true){
			j = rand()%7 + 1;
			if(j != butterflies[i].headColor){
				butterflies[i].wingColor = j;
				break;
			}
		}
		butterflies[i].x = ppoints[pindices[i]].x;
		butterflies[i].y = ppoints[pindices[i]].y;
	}
}

void drawButterfly (struct butterfly *bfly, uint8_t *canvas){
	Point *point = points + bfly->pointsOffset * POINTS_PER_BUTTERFLY;
	int i;
	color clrr;
	for(i=0;i<POINTS_PER_BUTTERFLY;i++){
		if((point + i) ->x ==1 && (point + i) ->y == 1){ // {1,1} the head
			clrr = bfly->headColor;
		}else{
			clrr = bfly->wingColor;
		}
		drawPixel((point + i) ->x + bfly->x, (point + i) ->y + bfly->y, clrr, canvas);
		//check if a butterfly head landed on a perch point
		if(clrr == bfly->headColor && bfly->moving) checkMove(bfly);
	}
}

// checks to see if a butterfly intersects any other current butterfly
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

// checks to see if two butterflies intersect each other
bool butterfliesIntersect(struct butterfly bflyA, struct butterfly bflyB){
	return (bflyA.x+BUTTERFLY_LENGTH >= bflyB.x && bflyB.x+BUTTERFLY_LENGTH >= bflyA.x &&
		bflyA.y+BUTTERFLY_LENGTH >= bflyB.y && bflyB.y+BUTTERFLY_LENGTH >= bflyA.y);
}

// Function that fills an array with non duplicate random numbers from 0-29.
// Can probably be done more efficiently but it works and doesn't require calling a help function
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

// Checks to see if a butterfly has landed on a perch point.
// If so, it sets its moving field to zero.
void checkMove(struct butterfly *bfly){
	int i;
	if(bfly->y == 11){ // y coordinate of all perch points
		for(i=0;i<10;i++){ // 10 perch points
			if(bfly->x == (i*6+3)){ // x offset of each perch point
				bfly->moving = 0;
				return;
			}
		}
	}

}

// self explanatory
void drawDoneMessage(uint8_t *canvas){
	char *mesg = "DONE!";
	int msglen = strlen(mesg);
	int i;
	for(i=0;i<msglen;i++){
		drawLetter(mesg[i], i*7+5, 2, red, canvas);
	}
}

// utility compare function to be used in qsort
int compare_points(const void* a, const void* b) {
	Point *pointA = (Point *)a;
	Point *pointB = (Point *)b;
	return pointA->x - pointB->x;
}

