#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NUMBER_ROWS 8
#define ROW_INT_LENGTH 8  // each row can contain up to 8 ints of bits (8 * sizeof(int) * 8 bits/byte) worth of text (42 letters max)
#define MAX_TEXT_LENGTH 42
#define NUMBER_COLUMNS_PER_PANEL 32
#define NUMBER_PANELS 1
#define LETTER_WIDTH 6
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

uint8_t _A[8][6] = {{0,0,1,0,0,0},{0,1,0,1,0,0},{1,0,0,0,1,0},{1,1,1,1,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{0,0,0,0,0,0}};
uint8_t _B[8][6] = {{1,1,1,1,0,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,1,1,1,0,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,1,1,1,0,0},{0,0,0,0,0,0}};
uint8_t _C[8][6] = {{0,1,1,1,0,0},{1,0,0,0,1,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,0,0,0,1,0},{0,1,1,1,0,0},{0,0,0,0,0,0}};
uint8_t _D[8][6] = {{1,1,1,1,0,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,1,1,1,0,0},{0,0,0,0,0,0}};
uint8_t _E[8][6] = {{1,1,1,1,1,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,1,1,1,0,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,1,1,1,1,0},{0,0,0,0,0,0}};
uint8_t _F[8][6] = {{1,1,1,1,1,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,1,1,1,0,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{0,0,0,0,0,0}};
uint8_t _G[8][6] = {{0,1,1,1,0,0},{1,0,0,0,1,0},{1,0,0,0,0,0},{1,0,0,1,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{0,1,1,1,1,0},{0,0,0,0,0,0}};
uint8_t _H[8][6] = {{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,1,1,1,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{0,0,0,0,0,0}};
uint8_t _I[8][6] = {{0,1,1,1,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,1,1,1,0,0},{0,0,0,0,0,0}};
uint8_t _J[8][6] = {{0,0,1,1,1,0},{0,0,0,1,0,0},{0,0,0,1,0,0},{0,0,0,1,0,0},{0,0,0,1,0,0},{1,0,0,1,0,0},{0,1,1,0,0,0},{0,0,0,0,0,0}};
uint8_t _K[8][6] = {{1,0,0,0,1,0},{1,0,0,1,0,0},{1,0,1,0,0,0},{1,1,0,0,0,0},{1,0,1,0,0,0},{1,0,0,1,0,0},{1,0,0,0,1,0},{0,0,0,0,0,0}};
uint8_t _L[8][6] = {{1,0,0,0,0,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,1,1,1,1,0},{0,0,0,0,0,0}};
uint8_t _M[8][6] = {{1,0,0,0,1,0},{1,1,0,1,1,0},{1,0,1,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{0,0,0,0,0,0}};
uint8_t _N[8][6] = {{1,0,0,0,1,0},{1,1,0,0,1,0},{1,0,1,0,1,0},{1,0,0,1,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{0,0,0,0,0,0}};
uint8_t _O[8][6] = {{0,1,1,1,0,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{0,1,1,1,0,0},{0,0,0,0,0,0}};
uint8_t _P[8][6] = {{1,1,1,1,0,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,1,1,1,0,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{1,0,0,0,0,0},{0,0,0,0,0,0}};
uint8_t _Q[8][6] = {{0,1,1,1,0,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,1,0,1,0},{1,0,0,1,0,0},{0,1,1,0,1,0},{0,0,0,0,0,0}};
uint8_t _R[8][6] = {{1,1,1,1,0,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,1,1,1,0,0},{1,0,1,0,0,0},{1,0,0,1,0,0},{1,0,0,0,1,0},{0,0,0,0,0,0}};
uint8_t _S[8][6] = {{0,1,1,1,0,0},{1,0,0,0,1,0},{1,0,0,0,0,0},{0,1,1,1,0,0},{0,0,0,0,1,0},{1,0,0,0,1,0},{0,1,1,1,0,0},{0,0,0,0,0,0}};
uint8_t _T[8][6] = {{1,1,1,1,1,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,0,0,0,0}};
uint8_t _U[8][6] = {{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{0,1,1,1,0,0},{0,0,0,0,0,0}};
uint8_t _V[8][6] = {{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{0,1,0,1,0,0},{0,0,1,0,0,0},{0,0,0,0,0,0}};
uint8_t _W[8][6] = {{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,1,0,1,0},{1,0,1,0,1,0},{0,1,0,1,0,0},{0,0,0,0,0,0}};
uint8_t _X[8][6] = {{1,0,0,0,1,0},{1,0,0,0,1,0},{0,1,0,1,0,0},{0,0,1,0,0,0},{0,1,0,1,0,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{0,0,0,0,0,0}};
uint8_t _Y[8][6] = {{1,0,0,0,1,0},{1,0,0,0,1,0},{1,0,0,0,1,0},{0,1,0,1,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,0,0,0,0}};
uint8_t _Z[8][6] = {{1,1,1,1,1,0},{0,0,0,0,1,0},{0,0,0,1,0,0},{0,0,1,0,0,0},{0,1,0,0,0,0},{1,0,0,0,0,0},{1,1,1,1,1,0},{0,0,0,0,0,0}};
uint8_t __[8][6] = {{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0}};
uint8_t excl[8][6] = {{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,1,0,0,0},{0,0,0,0,0,0},{0,0,1,0,0,0},{0,0,0,0,0,0}};


void* bitPacks[28] = {_A,_B,_C,_D,_E,_F,_G,_H,_I,_J,_K,_L,_M,_N,_O,_P,_Q,_R,_S,_T,_U,_V,_W,_X,_Y,_Z,__,excl};
char characters[28] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' ','!'};

void* getCharbits(char element){
	int i;
	for(i=0;i<28;i++){
		if(element == characters[i] )
			return bitPacks[i];
	}
	return __;
}

void setBitt(int *buffer, int offset){ //set the bit in the k-th position from the left in A
	int num = offset/32;
	buffer[num] |= (0x80000000 >> (offset - num * 32));
}

int getBit( int A, int k ){
	return ( (A & (0x80000000 >> k )) != 0 ) ;//get the bit in the k-th position from the left in A
}

void fillStringIntBuffer(int* intBuffer, char* str);

void fillPanel(int* intBuffer, int* messageString, int messageStringOffset, int row_length); 

void padAndfillPanel(int* intBuffer, int* messageString, int padding, int row_length);

int main(int argc, char **argv){
	int intArrayBuffer[64];
	memset(intArrayBuffer, 0, sizeof(intArrayBuffer));
	int *iptr = (int*)intArrayBuffer;
	char *str = "CUANDO ESTABA YO EN LA";
	fillStringIntBuffer(iptr , str);

	size_t str_length = strnlen(str, MAX_TEXT_LENGTH);

	int i, j, count;

	int bit_len = str_length*LETTER_WIDTH;

	for(i=0,count=0;i<NUMBER_ROWS;i++){
		for(j=0;j<bit_len;j++,count++){
			int val = getBit(intArrayBuffer[count/32], count%32);
			if(val == 0) 
				printf(" ");
			else
				printf("1");
		}
		printf("\n");
	} 


	int panelIntArray[NUMBER_ROWS] = {0,0,0,0,0,0,0,0};
        
	//        fillPanel(panelIntArray, intArrayBuffer, 12, bit_len);
	padAndfillPanel(panelIntArray, intArrayBuffer, 12, bit_len);
}

void fillPanel(int* intBuffer, int* messageString, int messageStringOffset, int row_length){

  int i,j,k,count;
  for(i=0,count=0;i<NUMBER_ROWS;i++){
    for(j=0,k=0;j<row_length;j++,count++){
      if((messageStringOffset==0 || j/messageStringOffset > 0) && k < 32 ){
	int val = getBit(messageString[count/32], count%32);
        if(val) intBuffer[i] |= (0x80000000 >> k);
	k++;
      }
    }
  }

  printf("\n");
}



void padAndfillPanel(int* intBuffer, int* messageString, int padding, int row_length){

  int i,j,count;
  for(i=0,count=0;i<NUMBER_ROWS;i++){
    for(j=0;j<row_length;j++,count++){
      if(j>=padding && j<32) {
        int val = getBit(messageString[(count-padding)/32], (count-padding)%32);
        if(val) {
	  intBuffer[i] |= (0x80000000 >> j);
          printf("1");
	}else
          printf(" ");
      }
      if(j<padding)
	printf(" ");
    }
  printf("\n");
  }

}



void fillStringIntBuffer(int *intBuffer, char* str){

	size_t str_length = strnlen(str, MAX_TEXT_LENGTH);
	void* text[str_length];
	int z,k,j,m;

	for(z=0; z< str_length; z++){
		text[z] = getCharbits(str[z]); //Load the input string str
	}

	int count = 0;
	for(z=0; z< NUMBER_ROWS-1; z++){ // for each row (8)
		for(j=0;j<str_length;j++){ // for each letter
			uint8_t (*arr)[LETTER_WIDTH] = (uint8_t (*)[LETTER_WIDTH]) text[j];
			for(m=0;m<LETTER_WIDTH;m++){
				//      	    if(count && count%6 == 0) printf(" ");
				if(arr[z][m]){
					setBitt(intBuffer, count);
					//				       printf(ANSI_COLOR_GREEN   "1");
				}else{
					// printf(" ");
				}
				count++;
			}
		}
		// printf(ANSI_COLOR_RESET  "\n");
	}


}
