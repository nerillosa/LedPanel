// letrero.c
// Uses bcm2835, a fast GPIO C library by Mike McCauley. I just use the basic output functionality.
// To install : http://www.raspberry-projects.com/pi/programming-in-c/io-pins/bcm2835-by-mike-mccauley
// After installing bcm2835, you can build and run this with:
// gcc -o letrero -l rt letrero.c lcdmatrix.c -l bcm2835
// sudo ./letrero
//
// Author: Neri Llosa (nerillosa@gmail.com)
// Copyright (C) 2016 Neri Llosa

#include <bcm2835.h>
#include <stdio.h>
#include <stdint.h>
#include "lcdmatrix.h"

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

void toggleClock(void);
void toggleLatch(void);

int main(int argc, char **argv)
{
    if (!bcm2835_init())
	return 1;

    int row1BitsArray[NUMBER_ROWS] = {0,0,0,0,0,0,0,0};
    getRows(row1BitsArray, NUMBER_ROWS, "TUMIA");

    // Set the pin to be an output
    bcm2835_gpio_fsel(AA, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(BB, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(CC, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(R1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(G1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(B1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(R2, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(G2, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(B2, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(CLK, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OE, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(LAT, BCM2835_GPIO_FSEL_OUTP);

    bcm2835_gpio_write(R1, LOW);
    bcm2835_gpio_write(B1, LOW);
    bcm2835_gpio_write(G1, LOW);
    bcm2835_gpio_write(R2, LOW);
    bcm2835_gpio_write(B2, LOW);
    bcm2835_gpio_write(G2, LOW);

    bcm2835_gpio_write(CLK, HIGH);
    bcm2835_gpio_write(LAT, LOW);

    uint8_t a,b,c,k;
    int i;

    while (1)
    {
	bcm2835_gpio_write(OE, HIGH); //disable output

        a = (k & 1)? HIGH : LOW; // change the row
        b = (k & 2)? HIGH : LOW;
        c = (k & 4)? HIGH : LOW;
        bcm2835_gpio_write(AA, a);
        bcm2835_gpio_write(BB, b);
        bcm2835_gpio_write(CC, c);

        int row1Bits = row1BitsArray[k%8];

	for(i=0;i<32;i++){
             	int yy = row1Bits & 0x80000000;
           	if(yy == 0){
			bcm2835_gpio_write(G1, LOW);
		}else{
			bcm2835_gpio_write(G1, HIGH);
		}

		row1Bits <<= 1;
		toggleClock();
	}

        toggleLatch();
        k++;
	bcm2835_gpio_write(OE, LOW); //enable output

        // give it a little time to display
        delay(1);
    }

    //bcm2835_close();

    return 0;
}

void toggleClock(){
     // Turn it on
     bcm2835_gpio_write(CLK, LOW);
     // wait a bit
     //delay(1);
     // turn it off
     bcm2835_gpio_write(CLK, HIGH);
//     delay(1);

}

void toggleLatch(){
     // Turn it on
     bcm2835_gpio_write(LAT, HIGH);
     // wait a bit
//     delay(1);
     // turn it off
     bcm2835_gpio_write(LAT, LOW);
//     delay(1);

}

