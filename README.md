# LedPanel
Moving Message LED Panel programming code and setup instructions.
## Synopsis
This projects describes how to put together LED panels and control them using a Raspberry Pi. The panels I am using are from adafruit.com and you can view their full description here: https://www.adafruit.com/products/420 .
In a nutshell, they are the so-called "1/8 scanning" panels. Pixels are divided in groups of 8 and are shown one at a time. The panels are 16x32 (16 rows and 32 columns). The 8 groups are arranged in rows: 1st and 9th, 2nd and 10th, 3rd and 11th, ..., 8th and 16th.
Each LED is basically 3 LEDS in one, as it has RGB components. This amounts to 16x32x3 = 1536 LEDS.
Each panel has a total of 12 control pins:
* A, B, and C. These 3 inputs define the current row group. 000-111 
* R1,G1,B1,R2,G2,B2. These are the color inputs for each LED. R1,G1,B1 control rows 1-8. R2,G2,B2 control rows 9-16. This makes it easy to divide the screen into two independent lines of text.
* OE, CLK, and LAT. These are pure control. The OE (output enable) enables/disables display when changing row group. It enables output with a LOW value. CLK is the Clock. This works on the negative edge and is used to shift the data inputs (R1,G1,B1,R2,G2,B2) 32 times for each row.
LAT is the Latch control. Once all shifts have been done for a row, this signals the panel to transfer the input data bits to the display output. Following is a diagram of the connections necessary to make this work. GPIO outputs do not necessarily have to be the ones depicted. Any free output port will do. The LED panel connector also has 3 GND pins and a D pin. All of them should be connected to the Raspberry Pi ground. The D pin is for 32x32 panels and is not used.  

![Alt text](images/connections.jpg?raw=true "Connections")
## Code Example

Show what the library does as concisely as possible, developers should be able to figure out **how** your project solves their problem by looking at the code example. Make sure the API you are showing off is obvious, and that your code is short and concise.

## Motivation

A short description of the motivation behind the creation and maintenance of the project. This should explain **why** the project exists.

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
