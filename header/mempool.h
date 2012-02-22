/* 	Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

*/
#ifndef _MEMPOOL_H___
#define _MEMPOOL_H___

/* The following section allows us to configure the memory pool by adjusting the size of the individual memory pools */

/* Small Pool */
#define BLOCKNUM_POOL_S 96 //number of blocks
#define BLOCKSIZE_POOL_S 4 //size of one block, the size of 1 unit is equal to the size of 1 int

/* Medium Pool */
#define BLOCKNUM_POOL_M 96 //number of blocks
#define BLOCKSIZE_POOL_M 8 //ints per block

/* Large Pool */
/* Ex. 64 blocks x 16 words = (64 x 32bytes) = 2048 bytes */
#define BLOCKNUM_POOL_L 64 //number of blocks
#define BLOCKSIZE_POOL_L 16 //words per block

void initMemory();
int *allocBlock(int size);
void freeBlock(int *arrayAddress);
#endif
