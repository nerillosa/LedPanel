#ifndef LCDMATRIX_H_   /* Include guard */
#define LCDMATRIX_H_

#define NUMBER_PANELS 2
#define LETTER_WIDTH 6
#define NUMBER_ROWS 8
#define NUMBER_COLUMNS_PER_PANEL 32
#define MAX_TEXT_LENGTH 200
#define INT_BITS sizeof(int) * 8
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void setBitR( int *A,  int k );
int  getBitR( int A,  int k );
void setBitL(int *buffer, int offset); //set the bit in the k-th position from the left in A
int  getBitL( int A, int k ); //get the bit in the k-th position from the left in A
void getRows(int *buff, int buffsize, char *str);
void fillPanel(int* intBuffer, int* messageString, int messageStringOffset, int row_length_in_bits); //fills a buffer that displays a whole panel
void padAndfillPanel(int* intBuffer, int* messageString, int padding, int row_length);
void fillStringIntBuffer(int *intBuffer, char* str); // fills a fixed sized buffer with string contents 

void shiftLeft(int rowBits[NUMBER_ROWS]);
void copyIntArrays(int *dest, int *src, int size);

#endif // LCDMATRIX_H_
