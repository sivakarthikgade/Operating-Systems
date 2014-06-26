#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/*
 * test.c
 *
 *  Created on: Apr 12, 2014
 *      Author: sivakarthik
 */

void main() {
	int i = 128;
	printf("i: %d\n",i);
	i = i >> 1;
	printf("i: %d\n",i);
	i = i >> 2;
	printf("i: %d\n",i);
	i = 128 | i;
	printf("i: %d\n",i);
}

//typedef union {
//	int mInstr;
//	float mData;
//} mSiva;
//mSiva *memory;
//
//typedef struct {
//	char isFree;
//	mSiva *memory;
//} mFrame;
//mFrame	*framesTable;
//
//void main() {
//	printf("Start of main.\n");
//	framesTable = malloc(2*sizeof(mFrame));
//	framesTable[0].isFree = 'n';
//	framesTable[0].memory = malloc(10*sizeof(mSiva));
//	framesTable[0].memory[0].mInstr = 5;
//	framesTable[0].memory[1].mInstr = 2;
//	framesTable[0].memory[2].mInstr = 76;
//	framesTable[0].memory[3].mInstr = 32;
//	framesTable[0].memory[4].mData = 11.1;
//	framesTable[0].memory[5].mData = 7.7;
////	framesTable[0].memory[6].mData = 21.27;
//	framesTable[1].isFree = 'y';
//
//	int i = 0;
//	for(i = 0; i < 2; i++) {
//		printf("Is frame[%d] free ? [%c]\n", i, framesTable[i].isFree);
//		printf("Size of frame[%d]: %lu\n", i, sizeof(framesTable[i].memory));
//	}
//	printf("End of main.\n");
//}
