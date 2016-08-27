# LedPanel
Moving Message LED Panel programming code and setup instructions.
## Synopsis
This projects describes how to put together LED panels and control them using a Raspberry Pi. The panels I am using are from adafruit.com and you can view their full description here: https://www.adafruit.com/products/420 .
In a nutshell, they are the so-called "1/8 scanning" panels. Pixels are divided in groups of 8 and are rendered one group at a time. The panels are 16x32 (16 rows and 32 columns). For this project, I am using two of them connected in series (one's ouptut connects to the other's input). You can also test with a single panel. The 8 groups are arranged in rows: 1st and 9th, 2nd and 10th, 3rd and 11th, ..., 8th and 16th.
Each LED is basically 3 LEDS in one, as it has RGB components. This amounts to 16x32x3 = 1536 LEDS.
Each panel has a total of 12 control pins:
* A, B, and C. These 3 inputs define the current row group. 000-111 
* R1,G1,B1,R2,G2,B2. These are the color inputs for each LED. R1,G1,B1 control rows 1-8. R2,G2,B2 control rows 9-16. This makes it easy to divide the screen into two independent lines of text.
* OE, CLK, and LAT. These are pure control. The OE (output enable) enables/disables display when changing row group. It enables output with a LOW value. CLK is the Clock. This works on the negative edge and is used to shift the data inputs (R1,G1,B1,R2,G2,B2) 32 (or 64 if you have two panels connected together) times for each row.
LAT is the Latch control. Once all shifts have been done for a row, this signals the panel to transfer the input data bits to the display output. Following is a diagram of the connections necessary to make this work. GPIO outputs do not necessarily have to be the ones depicted. Any free output port will do, just make sure you change the corresponding #define definitions in the header files. The LED panel connector also has 3 GND pins and a D pin. All of them should be connected to the Raspberry Pi ground. The D pin is for 32x32 panels and is not used.  

![Alt text](images/connections.jpg?raw=true "Connections")

![Alt text](images/connections2.jpg?raw=true "Connections2")

## Code Example

This is example code from movingShape.c. It depicts a smiley face running continuously from left to right. The code is very concise and the programming reduces to implementing the paintCanvas() function for your particular shape (smiley in this case). All the constants and draw shape functions are defined in lcdshapes.c. You can take a look at how they are implemented any time. The advantage of this project is that you get to see and analyze all the coding involved as there are no "hidden" libraries that give you no clue on how they work. The only external library used except for the standard C libraries is bcm2835, a GPIO C library by Mike McCauley. This is used to setup the GPIO pins as outputs and to control their output values. 
To compile and run the program you don't need any fancy Makefile or complicated commands. All you need to do is execute two commands. The first to compile the code and create the executable. The other to run the program (needs sudo rights).

gcc -o movingShape -l rt movingShape.c lcdshapes.c -l bcm2835 <br>
sudo ./movingShape

```c
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcdshapes.h"

#define MOVE_INTERVAL  150  // time in milliseconds between each move

void paintCanvas(uint8_t *canvas);

struct timeval saved, now;

int main(int argc, char **argv)
{
	if (gpio_init())
		return 1;

	uint8_t *canvas = (uint8_t *)calloc(PANEL_SIZE, sizeof(uint8_t));
	gettimeofday(&saved, NULL); //start time

	while (1) //infinite loop
	{
		displayRowInit();
		paintCanvas(canvas);
		updateRows(canvas);
	}
}

void paintCanvas(uint8_t *canvas){
	static uint16_t moveOffset = 0;
	gettimeofday(&now, NULL);
	if(getTimeDiff(now, saved) > MOVE_INTERVAL){
		timevalCopy(&saved, &now);
		memset(canvas, 0, PANEL_SIZE); // clear the canvas
		if(++moveOffset == (TOTAL_NUMBER_COLUMNS))
			moveOffset = 0;
		drawSmileyFace(moveOffset + 7, 8, 15, canvas); // smile!
	}
}
```
## Motivation

I delved in moving message panels back in 1997. There was no raspberry pi then. I used a 80C31 microcontroller and the program was written directly in assembly language. There were no tri-color LEDs either. I used a 8x80 panel constructed with red Leds and cardboard backing. I had to add all the LED driver hardware and their controller chips. This included shift registers, latches, NAND gates, power transistors, etc. It was fascinating and challenging work then and still is to this day. Today, the panels come with all their support hardware included so the low current, 3.3 V outputs of a Raspberry Pi are enough to drive them. All you have to do is connect 12 wires and a power supply, add some software and the rpi (with "unlimited" memory as opposed to the 64k of thhe 80C31), and you're good to go.

## Installation

Provide code examples and explanations of how to get the project.

## API Reference

Depending on the size of the project, if it is small and simple enough the reference docs can be added to the README. For medium size to larger projects it is important to at least provide a link to where the API reference docs live.

## Tests

Describe and show how to run the tests with code examples.

## Contributors

Let people know how they can dive into the project, include important links to things like issue trackers, irc, twitter accounts if applicable.

## License

A short snippet describing the license (MIT, Apache, etc.)
