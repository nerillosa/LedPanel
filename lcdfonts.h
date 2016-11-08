#ifndef LCDFONTS_H_   /* Include guard */
#define LCDFONTS_H_

#define LETTER_WIDTH 7
#define LETTER_HEIGHT 7

#define AF1 {0x78,0xCC,0xCC,0xCC,0xFC,0xCC,0xCC}
#define BF1 {0xF8,0xCC,0xCC,0xF8,0xCC,0xCC,0xF8}
#define CF1 {0x78,0xCC,0xC0,0xC0,0xC0,0xCC,0x78}
#define DF1 {0xF8,0xCC,0xCC,0xCC,0xCC,0xCC,0xF8}
#define EF1 {0xFC,0xC0,0xC0,0xF8,0xC0,0xC0,0xFC}
#define FFF1 {0xFC,0xC0,0xC0,0xF8,0xC0,0xC0,0xC0}
#define GF1 {0x78,0xCC,0xC0,0xC0,0xDC,0xCC,0x7C}
#define HF1 {0xCC,0xCC,0xCC,0xFC,0xCC,0xCC,0xCC}
#define IF1 {0x78,0x30,0x30,0x30,0x30,0x30,0x78}
#define JF1 {0x0C,0x0C,0x0C,0x0C,0x0C,0xCC,0x78}
#define KF1 {0xCC,0xD8,0xF0,0xE0,0xF0,0xD8,0xCC}
#define LF1 {0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFC}
#define MF1 {0xC4,0xEC,0xFC,0xD4,0xC4,0xC4,0xC4}
#define NF1 {0xCC,0xCC,0xEC,0xFC,0xDC,0xCC,0xCC}
#define OF1 {0x78,0xCC,0xCC,0xCC,0xCC,0xCC,0x78}
#define PF1 {0xF8,0xCC,0xCC,0xCC,0xF8,0xC0,0xC0}
#define QF1 {0x78,0xC8,0xC8,0xC8,0xC8,0xD8,0x7C}
#define RF1 {0xF8,0xCC,0xCC,0xF8,0xF0,0xD8,0xCC}
#define SF1 {0x78,0xCC,0xC0,0x78,0x0C,0xCC,0x78}
#define TF1 {0xFC,0x30,0x30,0x30,0x30,0x30,0x30}
#define UF1 {0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x78}
#define VF1 {0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x30}
#define WF1 {0xC4,0xC4,0xC4,0xC4,0xD4,0x7C,0x28}
#define XF1 {0x64,0x64,0x3C,0x18,0x3C,0x64,0x64}
#define YF1 {0xCC,0xCC,0xCC,0x78,0x30,0x30,0x30}
#define ZF1 {0x7C,0x64,0x0C,0x18,0x30,0x64,0x7C}
#define N0F1 {0x78,0xCC,0xCC,0xDC,0xEC,0xCC,0x78}
#define N1F1 {0x10,0x30,0x70,0x30,0x30,0x30,0x78}
#define N2F1 {0x78,0xCC,0x0C,0x18,0x30,0x60,0xFC}
#define N3F1 {0x78,0xCC,0x0C,0x18,0x0C,0xCC,0x78}
#define N4F1 {0x40,0xC0,0xD8,0xD8,0xFC,0x18,0x18}
#define N5F1 {0xFC,0xC0,0xC0,0xF8,0x0C,0xCC,0x78}
#define N6F1 {0x78,0xCC,0xC0,0xF8,0xCC,0xCC,0x78}
#define N7F1 {0xFC,0x0C,0x0C,0x18,0x30,0x30,0x30}
#define N8F1 {0x78,0xCC,0xCC,0x78,0xCC,0xCC,0x78}
#define N9F1 {0x78,0xCC,0xCC,0x7C,0x0C,0xCC,0x78}

#define NEXCF1 {0x0C,0x0C,0x0C,0x0C,0x0C,0x00,0x0C} /* ! */
#define NQSTF1 {0x78,0xCC,0x0C,0x38,0x30,0x00,0x30} /* ? */
#define NCOMMAF1 {0x00,0x00,0x00,0x00,0x00,0x40,0xC0}   /* COMMA */
#define NDASHF1 {0x00,0x00,0x00,0x78,0x00,0x00,0x00}   /* DASH */
#define NAPOSTRF1 {0x30,0x30,0x00,0x00,0x00,0x00,0x00}   /* APOSTROPHE */
#define NCOLONF1 {0x00,0x10,0x10,0x00,0x10,0x10,0x00}   /* COLON */
#define NPERIODF1 {0x00,0x00,0x00,0x00,0x00,0x00,0x40}   /* PERIOD */
#define NSCOLONF1 {0x00,0x10,0x10,0x00,0x30,0x10,0x00}   /* SEMI COLON */
#define N_F1 {0x00,0x00,0x00,0x00,0x00,0x00,0x00}   /* SPACE */

#define LETTERS {AF1,BF1,CF1,DF1,EF1,FFF1,GF1,HF1,\
	IF1,JF1,KF1,LF1,MF1,NF1,OF1,PF1,QF1,RF1,SF1,TF1,\
	UF1,VF1,WF1,XF1,YF1,ZF1,N0F1,N1F1,N2F1,N3F1,N4F1,\
	N5F1,N6F1,N7F1,N8F1,N9F1,NEXCF1,NQSTF1,NCOMMAF1,NDASHF1,\
	NAPOSTRF1,NCOLONF1,NPERIODF1,NSCOLONF1,N_F1}

#define GET_ALPHA(L) \
	(L == 'A') ? letters[0] : (L == 'B') ? letters[1] :\
	(L == 'C') ? letters[2] : (L == 'D') ? letters[3] :\
	(L == 'E') ? letters[4] : (L == 'F') ? letters[5] :\
	(L == 'G') ? letters[6] : (L == 'H') ? letters[7] :\
	(L == 'I') ? letters[8] : (L == 'J') ? letters[9] :\
	(L == 'K') ? letters[10] : (L == 'L') ? letters[11] :\
	(L == 'M') ? letters[12] : (L == 'N') ? letters[13] :\
	(L == 'O') ? letters[14] : (L == 'P') ? letters[15] :\
	(L == 'Q') ? letters[16] : (L == 'R') ? letters[17] :\
	(L == 'S') ? letters[18] : (L == 'T') ? letters[19] :\
	(L == 'U') ? letters[20] : (L == 'V') ? letters[21] :\
	(L == 'W') ? letters[22] : (L == 'X') ? letters[23] :\
	(L == 'Y') ? letters[24] : (L == 'Z') ? letters[25] :\
	(L == '0') ? letters[26] : (L == '1') ? letters[27] :\
	(L == '2') ? letters[28] : (L == '3') ? letters[29] :\
	(L == '4') ? letters[30] : (L == '5') ? letters[31] :\
	(L == '6') ? letters[32] : (L == '7') ? letters[33] :\
	(L == '8') ? letters[34] : (L == '9') ? letters[35] :\
	(L == '!') ? letters[36] : (L == '?') ? letters[37] :\
	(L == ',') ? letters[38] : (L == '-') ? letters[39] :\
	(L == '\'') ? letters[40] : (L == ':') ? letters[41] :\
	(L == '.') ? letters[42] : (L == ';') ? letters[43] :\
	letters[44]



#endif // LCDFONTS_H_
