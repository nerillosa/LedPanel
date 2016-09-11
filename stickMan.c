// stickMan.c
// Running this program shows a stickMan waving
// After installing bcm2835, you can build and run this with:
// gcc -o stickMan -l rt stickMan.c lcdshapes.c -l bcm2835
// sudo ./stickMan

#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"
#include "lcdfonts.h"

#define MOVE_INTERVAL  150  // time in milliseconds between each move
#define X_POSITION 30 // x position of the stickMan
#define STICKMAN_WIDTH 10

void paintCanvas(uint8_t *canvas);
void setRectanglePoints(int x, int y, int width, int height);

struct timeval saved, now;

// Five stickman figures 10 pixels wide standing side by side
// in various stages of waving.
// Created using pixels.html
Point points[] = {{3,1},{4,1},{5,1},{13,1},{14,1},{15,1},{23,1},{24,1},{25,1},
{33,1},{34,1},{35,1},{43,1},{44,1},{45,1},{2,2},{6,2},{12,2},{16,2},{22,2},{26,2},
{32,2},{36,2},{42,2},{46,2},{2,3},{6,3},{12,3},{16,3},{22,3},{26,3},{32,3},{36,3},
{42,3},{46,3},{3,4},{4,4},{5,4},{13,4},{14,4},{15,4},{23,4},{24,4},{25,4},{33,4},
{34,4},{35,4},{43,4},{44,4},{45,4},{48,4},{4,5},{14,5},{24,5},{34,5},{38,5},{44,5},
{47,5},{4,6},{14,6},{24,6},{34,6},{36,6},{37,6},{44,6},{46,6},{3,7},{4,7},{5,7},
{13,7},{14,7},{15,7},{23,7},{24,7},{25,7},{26,7},{27,7},{28,7},{33,7},{34,7},{35,7},
{43,7},{44,7},{45,7},{2,8},{4,8},{6,8},{12,8},{14,8},{16,8},{17,8},{22,8},{24,8},
{32,8},{34,8},{42,8},{44,8},{1,9},{4,9},{7,9},{11,9},{14,9},{18,9},{21,9},{24,9},
{31,9},{34,9},{41,9},{44,9},{0,10},{4,10},{8,10},{10,10},{14,10},{20,10},{24,10},
{30,10},{34,10},{40,10},{44,10},{4,11},{14,11},{24,11},{34,11},{44,11},{3,12},{5,12},
{13,12},{15,12},{23,12},{25,12},{33,12},{35,12},{43,12},{45,12},{2,13},{6,13},{12,13},
{16,13},{22,13},{26,13},{32,13},{36,13},{42,13},{46,13},{1,14},{7,14},{11,14},{17,14},
{21,14},{27,14},{31,14},{37,14},{41,14},{47,14}};

int len = sizeof(points)/sizeof(Point);

int main(int argc, char **argv)
{
	if (gpio_init()) return 1;
	uint8_t *canvas = (uint8_t *)calloc(PANEL_SIZE, sizeof(uint8_t));
	gettimeofday(&saved, NULL); //start time
	while (true) //infinite loop
	{
		displayRowInit();
		paintCanvas(canvas);
		updateRows(canvas);
	}
}

void paintCanvas(uint8_t *canvas){
	static int stickMan = 0; // start with first stickMan
	gettimeofday(&now, NULL);

        if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		setRectanglePoints(stickMan*STICKMAN_WIDTH, 0 , STICKMAN_WIDTH, NUMBER_ROWS);
		int i;
		int xOffset = X_POSITION - stickMan*STICKMAN_WIDTH;
		for(i=0;i<len;i++){
			if(points[i].c)
				drawPixel(points[i].x + xOffset, points[i].y, points[i].c, canvas);
		}
		if(++stickMan == 5){
			stickMan = 0; //repeat
		}
	}
}

// Makes points inside a rectangle visible
void setRectanglePoints (int x, int y, int width, int height){
	int i;
	for(i=0;i<len;i++){
		if(points[i].x >= x && points[i].x < x + width && points[i].y >= y && points[i].y < y + height){
			points[i].c = red;
		}else{
			points[i].c = 0;
		}
	}
}
