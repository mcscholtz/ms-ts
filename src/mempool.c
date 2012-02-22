/*  Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

	This file contains the functions for allocating and de-allocating memory. It also contains the 
	functions to initialize the memorymap (to cache the memory into control blocks)

*/
#include <p33FJ128GP802.h>
#include "memoryMap.h"
#include "mempool.h"
#define NULL (void *)0

typedef struct memoryBlock {
		int *arrayAddress; 
		int pool_id;
        struct memoryBlock *Next;
}memoryBlock;

/********************** Small Pool *********************************************************/
	/* Pointers to the top and bottom of free items for this pool */
static memoryBlock *Pool_head_free_s;
static memoryBlock *Pool_tail_free_s;

	/* The memory that will be dynamically allocated will be stored in this array */
static int pool_data_array_s[BLOCKNUM_POOL_S][BLOCKSIZE_POOL_S];       
	/* This is a array that contains all memory blocks for this pool */
static int pool_control_block_array_s[BLOCKNUM_POOL_S][sizeof(memoryBlock)/sizeof(int)]; 

/********************** Medium Pool *********************************************************/
static memoryBlock *Pool_tail_free_m;
static memoryBlock *Pool_tail_free_m;

static int pool_data_array_m[BLOCKNUM_POOL_M][BLOCKSIZE_POOL_M];    
static int pool_control_block_array_m[BLOCKNUM_POOL_M][sizeof(memoryBlock)/sizeof(int)];					

/********************** Large Pool *********************************************************/
static memoryBlock *Pool_head_free_l;
static memoryBlock *Pool_tail_free_l;

static int pool_data_array_l[BLOCKNUM_POOL_L][BLOCKSIZE_POOL_L]; 
static int pool_control_block_array_l[BLOCKNUM_POOL_L][sizeof(memoryBlock)/sizeof(int)];                          

/* This is a pointer to the root of the red-black Tree */
static struct rb_tree *memoryMap;
static int rootArray[sizeof(struct rb_tree)/sizeof(int)];

void initMemory();
void initPool_S();
void initPool_M();
void initPool_L();
void initUsed();
void freeBlock(int *block);
int *allocBlock(int size);

void initMemory()
{
	nodeCounter = 0;  
	memoryMap = (struct rb_tree *)&rootArray[0];
	/* Map items into the binary tree, populate free lists */
	initPool_S();
	initPool_M();
	initPool_L();
}

void initPool_S(){
  int i;  
  Pool_tail_free_s = NULL;
  Pool_head_free_s = NULL;
  for(i=0;i<BLOCKNUM_POOL_S;i++){  
      /* For each block setup the memory block and pointers	*/  
      if(Pool_tail_free_s){    
		  insert_node( memoryMap, (int *)&pool_data_array_s[i][0], (int *)&pool_control_block_array_s[i][0]);      
          Pool_tail_free_s->Next = (memoryBlock *)&pool_control_block_array_s[i][0];
          Pool_tail_free_s->Next->arrayAddress = (int *)&pool_data_array_s[i][0];		  
          Pool_tail_free_s->Next->pool_id = 1;
		  Pool_tail_free_s = Pool_tail_free_s->Next;
          Pool_tail_free_s->Next = NULL;
      }else{
		  insert_node( memoryMap, (int *)&pool_data_array_s[i][0], (int *)&pool_control_block_array_s[i][0]);
          Pool_head_free_s = (memoryBlock *)&pool_control_block_array_s[i][0];
          Pool_head_free_s->arrayAddress = (int *)&pool_data_array_s[i][0];  
		  Pool_head_free_s->pool_id = 1;
          Pool_head_free_s->Next = NULL;
          Pool_tail_free_s = Pool_head_free_s;
		}
	}
}

void initPool_M(){
  int i;
  Pool_tail_free_m = NULL;
  Pool_tail_free_m = NULL;
  for(i=0;i<BLOCKNUM_POOL_M;i++){
        /* For each block setup the memory block and pointers */
        if(Pool_tail_free_m){
			insert_node( memoryMap, (int *)&pool_data_array_m[i][0], (int *)&pool_control_block_array_m[i][0]);
            Pool_tail_free_m->Next = (memoryBlock *)&pool_control_block_array_m[i][0];
            Pool_tail_free_m->Next->arrayAddress = (int *)&pool_data_array_m[i][0];
			Pool_tail_free_m->Next->pool_id = 2;
            Pool_tail_free_m = Pool_tail_free_m->Next;
            Pool_tail_free_m->Next = NULL;
        }else{
			insert_node( memoryMap, (int *)&pool_data_array_m[i][0], (int *)&pool_control_block_array_m[i][0]);
            Pool_tail_free_m = (memoryBlock *)&pool_control_block_array_m[i][0];
            Pool_tail_free_m->arrayAddress = (int *)&pool_data_array_m[i][0];
			Pool_tail_free_m->pool_id = 2;
            Pool_tail_free_m->Next = NULL;
            Pool_tail_free_m = Pool_tail_free_m; 
        }      
    }
}

void initPool_L(){
  int i;
  Pool_tail_free_l = NULL;
  for(i=0;i<BLOCKNUM_POOL_L;i++){      
        /* For each block setup the memory block and pointers */
        if(Pool_tail_free_l){
			insert_node( memoryMap, (int *)&pool_data_array_l[i][0], (int *)&pool_control_block_array_l[i][0]);
            Pool_tail_free_l->Next = (memoryBlock *)&pool_control_block_array_l[i][0];
            Pool_tail_free_l->Next->arrayAddress = (int *)&pool_data_array_l[i][0];
			Pool_tail_free_l->Next->pool_id = 3;
            Pool_tail_free_l = Pool_tail_free_l->Next;
            Pool_tail_free_l->Next = NULL;
        }else{
		    insert_node( memoryMap, (int *)&pool_data_array_l[i][0], (int *)&pool_control_block_array_l[i][0]);
            Pool_head_free_l = (memoryBlock *)&pool_control_block_array_l[i][0];
            Pool_head_free_l->arrayAddress = (int *)&pool_data_array_l[i][0];
			Pool_head_free_l->pool_id = 3;
            Pool_head_free_l->Next = NULL;
            Pool_tail_free_l = Pool_head_free_l;
        }      
    }
}

int *allocBlock(int size){
    size = size/sizeof(int);
	int *Address;
	if((size <= BLOCKSIZE_POOL_S) && Pool_head_free_s){
		Address = Pool_head_free_s->arrayAddress;
		Pool_head_free_s = Pool_head_free_s->Next;
		return Address;
	}else if(size <= BLOCKSIZE_POOL_M && Pool_tail_free_m){
		Address = Pool_tail_free_m->arrayAddress;
		Pool_tail_free_m = Pool_tail_free_m->Next;
		return Address;
	}else if(size <= BLOCKSIZE_POOL_L && Pool_head_free_l){
		Address = Pool_head_free_l->arrayAddress;
		Pool_head_free_l = Pool_head_free_l->Next;
		return Address;
	}else{
		/* There was a problem Allocating memory */
		return NULL;
		} 
}

void freeBlock(int *arrayAddress){
    /* 	
		1 - Lookup arrayAddress on binary tree
			- Tree returns block address
			- Block contains pool_id
		2 - Add block to tail of free list
	*/
	memoryBlock *block_address;
	block_address = (memoryBlock *)searchTree(memoryMap, arrayAddress);

	switch(block_address->pool_id){
		case 1:			
			if(Pool_head_free_s){
				Pool_tail_free_s->Next = block_address;
				Pool_tail_free_s = block_address;
				Pool_tail_free_s->Next = NULL;
			}else{
				Pool_tail_free_s = block_address;
				Pool_tail_free_s->Next = NULL;
				Pool_head_free_s = Pool_tail_free_s;
			}
			break;
		case 2:			
			if(Pool_tail_free_m){
				Pool_tail_free_m->Next = block_address;
				Pool_tail_free_m = block_address;
				Pool_tail_free_m->Next = NULL;
			}else{
				Pool_tail_free_m = block_address;
				Pool_tail_free_m->Next = NULL;
				Pool_tail_free_m = Pool_tail_free_m;
			}
			break;
		case 3:	
			if(Pool_head_free_l){
				Pool_tail_free_l->Next = block_address;
				Pool_tail_free_l = block_address;
				Pool_tail_free_l->Next = NULL;
			}else{
				Pool_tail_free_l = block_address;
				Pool_tail_free_l->Next = NULL;
				Pool_head_free_l = Pool_tail_free_l;
			}
			break;
	}
}	
