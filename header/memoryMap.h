/* 	Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

*/
#ifndef _MEMORYMAP_H___
#define _MEMORYMAP_H___

struct node {   
	int red; /* 1 is red, 0 is black */
	int *blockAddress;		
    int *arrayAddress;
    struct node *link[2]; /* index 0 is left and index 1 is right */
};

struct rb_tree {
  struct node *root;
};
static int nodeCounter; //counts how many nodes have been allocated, used during init

int *searchTree(struct rb_tree *tree, int *arrayAddress);
int insert_node ( struct rb_tree *tree, int *arrayAddress, int *blockAddress );

#endif
