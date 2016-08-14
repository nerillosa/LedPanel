// letrero.c
//
// After installing bcm2835, you can build this
// with something like:
// gcc -o letrero -l rt letrero_lm.c lcdmatrix.c -l bcm2835
// sudo ./letrero
//
// Author: Neri Llosa (nerillosa@gmail.com)
// Copyright (C) 2016 Neri Llosa

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <bcm2835.h>

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
void shiftLeft(int rowBits[NUMBER_ROWS]);
void copyIntArrays(int *dest, int *src, int size);

int main(int argc, char **argv)
{
    if (!bcm2835_init())
	return 1;

    int intArrayBuffer[64];
    memset(intArrayBuffer, 0, sizeof(intArrayBuffer));

    int *iptr = (int*)intArrayBuffer;
    char *str = "CUANDO ESTABA YO EN LA CARCEL YO SOLITO";
    fillStringIntBuffer(iptr , str);

    size_t str_length = strnlen(str, MAX_TEXT_LENGTH);
    int bit_len = str_length * LETTER_WIDTH;

    int row1BitsArray[NUMBER_ROWS];
    memset(row1BitsArray, 0, sizeof(row1BitsArray));

    fillPanel(row1BitsArray, intArrayBuffer, 0, bit_len);

    int row2BitsArray[NUMBER_ROWS] = {0,0,0,0,0,0,0,0};
    getRows(row2BitsArray, NUMBER_ROWS, "LIFE!");

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

    uint8_t a,b,c,k=0;
    int i;
    int t = 0;
    int state = 0;
    while (1)
    {
	bcm2835_gpio_write(OE, HIGH); //disable output

        a = (k & 1)? HIGH : LOW; // change the row
        b = (k & 2)? HIGH : LOW;
        c = (k & 4)? HIGH : LOW;
        bcm2835_gpio_write(AA, a);
        bcm2835_gpio_write(BB, b);
        bcm2835_gpio_write(CC, c);

        if((k | 0x80) == 0xFF){ // shift once to the left every 128 cycles. k goes from 0-FF
	        if(state == 0){
                	memset(row1BitsArray, 0, sizeof(row1BitsArray));
               		fillPanel(row1BitsArray, intArrayBuffer, t, bit_len);
                	if(++t == bit_len + 1){
				t=0;
                                state = 1;
			}
		}else{
			memset(row1BitsArray, 0, sizeof(row1BitsArray));
			padAndfillPanel(row1BitsArray, intArrayBuffer, 31 - t, bit_len);
                        if(++t == 31){
				t=0;
				state = 0;
			}
		}
        }

        int row1Bits = row1BitsArray[k%8];
        int row2Bits = row2BitsArray[k%8];

	for(i=0;i<32;i++){
             	int yy = row1Bits & 0x80000000;
           	if(yy == 0){
			bcm2835_gpio_write(R1, LOW);
		}else{
			bcm2835_gpio_write(R1, HIGH);
		}

                yy = row2Bits & 0x80000000;
                if(yy == 0){
                        bcm2835_gpio_write(G2, LOW);
                }else{
                        bcm2835_gpio_write(G2, HIGH);
                }
		row1Bits <<= 1;
		row2Bits <<= 1;
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

void shiftLeft(int rowBits[NUMBER_ROWS]){
	int i;
	for(i=0;i<NUMBER_ROWS;i++){

		rowBits[i] =  rowBits[i] << 1 ;
	}
}

void copyIntArrays(int *dest, int *src, int size){
    int w;
    for(w=0 ; w<size ; w++){
        dest[w] = src[w];
    }
}
