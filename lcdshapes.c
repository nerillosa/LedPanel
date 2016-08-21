#include "lcdshapes.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int PANEL_SIZE = NUMBER_ROWS * NUMBER_PANELS * NUMBER_COLUMNS_PER_PANEL;
int TOTAL_NUMBER_COLUMNS = NUMBER_PANELS * NUMBER_COLUMNS_PER_PANEL;

void drawPixel(int x, int y, color c, uint8_t *display){
        if(x<0 || x>=TOTAL_NUMBER_COLUMNS || y<0 || y>=NUMBER_ROWS) return; //sanity check
        int offset = y * TOTAL_NUMBER_COLUMNS + x;
        *(display + offset) = c;
}

void drawHorizontalLine(int x, int y, int width, color c, uint8_t *display){
        if(x<0 || x>=TOTAL_NUMBER_COLUMNS || y<0 || y>=NUMBER_ROWS || width <=0) return; //sanity check
        int i, offset = y * TOTAL_NUMBER_COLUMNS + x;
        for(i=0;i<width;i++){
                if((offset + i) >= y*TOTAL_NUMBER_COLUMNS && (offset + i) < (y+1)*TOTAL_NUMBER_COLUMNS) // keep it in the same row
                        *(display + offset + i) = c;
        }
}

void drawVerticalLine(int x, int y, int height, color c, uint8_t *display){
        if(x<0 || x>=TOTAL_NUMBER_COLUMNS || y<0 || y>=NUMBER_ROWS || height <=0) return; //sanity check
        int i, j, offset = y * TOTAL_NUMBER_COLUMNS + x;
        for(i=0;i<height;i++){
                j = offset + i * TOTAL_NUMBER_COLUMNS;
                if(j >= 0 && j < PANEL_SIZE) // safety check just in case
                        *(display + j) = c;
        }
}

void drawSlantLine (int x, int y, bool isPositiveAngle, int length, color c, uint8_t *display){
        int i;
        for(i=0;i<length;i++){
                drawPixel(x + i,  isPositiveAngle ? (y-i): (y+i), c, display);
        }
}


void drawRectangle(int x, int y, int width, int height, color c, uint8_t *display){
        drawHorizontalLine(x, y, width, c, display);
        drawVerticalLine(x, y, height, c, display);
        drawHorizontalLine(x, y + height - 1, width, c, display);
        drawVerticalLine(x + width -1, y, height, c, display);
}

void drawCircle(int x, int y, int diameter, color c,  uint8_t *display){
        if(diameter == 15){
                drawVerticalLine(x-7, y-2, 5, c, display);
                drawVerticalLine(x+7, y-2, 5, c, display);
                drawHorizontalLine(x-2, y-7, 5, c, display);
                drawHorizontalLine(x-2, y+7, 5, c, display);

                drawSlantLine (x-6, y-4, true, 3, c, display);
                drawSlantLine (x+4, y+6, true, 3, c, display);
                drawSlantLine (x+4, y-6, false, 3, c, display);
                drawSlantLine (x-6, y+4, false, 3, c, display);

                drawPixel(x-6, y-3, c, display);
                drawPixel(x-3, y-6, c, display);
                drawPixel(x+3, y-6, c, display);
                drawPixel(x+6, y-3, c, display);
                drawPixel(x-6, y+3, c, display);
                drawPixel(x-3, y+6, c, display);
                drawPixel(x+3, y+6, c, display);
                drawPixel(x+6, y+3, c, display);
        }
}

void drawSmileyFace(int x, int y, int diameter, uint8_t *display){
        drawCircle(x, y, diameter, yellow, display);

        //mouth
        drawHorizontalLine(x-2, y+4, 5, red, display);
        drawPixel(x-3,y+3,red, display);
        drawPixel(x+3,y+3,red, display);

        //eyes
        drawHorizontalLine(x-3, y-3, 2, blue, display);
        drawHorizontalLine(x+2, y-3, 2, blue, display);

        //nose
        drawPixel(x, y-1, green, display);
        drawHorizontalLine(x-1, y, 3, green, display);
}


void updateRows(uint16_t count, uint8_t *displayArray){

        uint8_t *row1 = displayArray + (count%8 * TOTAL_NUMBER_COLUMNS);
        uint8_t *row2 = displayArray + ((count%8 + 8) * TOTAL_NUMBER_COLUMNS);
        int i;
        uint8_t val;
        for(i=0;i<TOTAL_NUMBER_COLUMNS;i++){
                val = *(row1+i);
                bcm2835_gpio_write(R1, ((val & 4)? HIGH : LOW));
                bcm2835_gpio_write(G1, ((val & 2)? HIGH : LOW));
                bcm2835_gpio_write(B1, ((val & 1)? HIGH : LOW));

                val = *(row2+i);
                bcm2835_gpio_write(R2, ((val & 4)? HIGH : LOW));
                bcm2835_gpio_write(G2, ((val & 2)? HIGH : LOW));
                bcm2835_gpio_write(B2, ((val & 1 )? HIGH : LOW));

                toggleClock(); // negative edge clock pulse
        }
}

int gpio_init(){

        if (!bcm2835_init())
                return 1;

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

        return 0;
}

void toggleClock(){
        bcm2835_gpio_write(CLK, LOW);
        bcm2835_gpio_write(CLK, HIGH);
        //delay(1);
}

void displayRowInit(uint8_t count){
        bcm2835_gpio_write(OE, HIGH); //disable output with a high value
	uint8_t a, b, c;
        a = (count & 1)? HIGH : LOW; // change the row
        b = (count & 2)? HIGH : LOW;
        c = (count & 4)? HIGH : LOW;
        bcm2835_gpio_write(AA, a);
        bcm2835_gpio_write(BB, b);
        bcm2835_gpio_write(CC, c);
}

void displayRowEnd(){
        // once all bits are done shifting, this LAT control transfers the data bits to the LED drivers (a la D-FlipFlop)
        bcm2835_gpio_write(LAT, HIGH);
        bcm2835_gpio_write(LAT, LOW);

        bcm2835_gpio_write(OE, LOW); //enable output

        // give it a little time to display
        delay(1);
}
