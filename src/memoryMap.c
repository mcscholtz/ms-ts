/*  Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

	***NOTICE***
	This file implements a red-black binary tree to map the memory addresses. 
	Codes was derived from the tutorial found here:
	
	http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx
	
*/
#include "memoryMap.h"
#include "mempool.h"
#define NULL (void *)0

/* This array will hold all of the binary tree's nodes */
static int treeArray[BLOCKNUM_POOL_S+BLOCKNUM_POOL_M+BLOCKNUM_POOL_L][sizeof(struct node)/sizeof(int)];

int is_red ( struct node *root );
struct node *single_rotate ( struct node *root, int dir );
struct node *double_rotate ( struct node *root, int dir );
struct node *make_node ( int *arrayAddress , int *blockAddress);
int insert_node ( struct rb_tree *tree, int *arrayAddress, int *blockAddress );
int *searchTree(struct rb_tree *tree, int *arrayAddress);

int *searchTree(struct rb_tree *tree, int *arrayAddress)
{
	struct node *tmpNode = tree->root;
	while(1){
		if(tmpNode->arrayAddress > arrayAddress){
			tmpNode = tmpNode->link[0];
		}else if(tmpNode->arrayAddress < arrayAddress){
			tmpNode = tmpNode->link[1];
		}else{
			return tmpNode->blockAddress;
		}
	}
}

int is_red ( struct node *root )
{
  return (root != NULL && root->red == 1);
}

struct node *single_rotate ( struct node *root, int dir )
 {
   struct node *save = root->link[!dir]; 
   root->link[!dir] = save->link[dir];
   save->link[dir] = root;
   root->red = 1;
   save->red = 0;
   return save;
 }
 
struct node *double_rotate ( struct node *root, int dir )
{
   root->link[!dir] = single_rotate ( root->link[!dir], !dir );
  return single_rotate ( root, dir );
}

struct node *make_node ( int *arrayAddress, int *blockAddress )
{
  struct node *rn = (struct node *)&treeArray[nodeCounter][0];
  nodeCounter++;
  if ( rn != NULL ) {
    rn->arrayAddress = arrayAddress;
	rn->blockAddress = blockAddress;
    rn->red = 1; /* 1 is red, 0 is black */
    rn->link[0] = NULL;
    rn->link[1] = NULL;
  }
  return rn;
}

int insert_node ( struct rb_tree *tree, int *arrayAddress, int *blockAddress )
{
  if ( tree->root == NULL ) {
    /* Empty tree case */
    tree->root = make_node ( arrayAddress , blockAddress);
     if ( tree->root == NULL )
      return 0;
  }
  else {
    struct node head = {0}; /* False tree root */
    struct node *g, *t;     /* Grandparent & parent */
    struct node *p, *q;     /* Iterator & parent */
    int dir = 0, last;
    /* Set up helpers */
    t = &head;
    g = p = NULL;
    q = t->link[1] = tree->root;
    /* Search down the tree */
    while(1) {
      if ( q == NULL ) {
        /* Insert new node at the bottom */
        p->link[dir] = q = make_node ( arrayAddress, blockAddress );
        if ( q == NULL )
          return 0;
      }
      else if ( is_red ( q->link[0] ) && is_red ( q->link[1] ) ) {
        /* Color flip */
        q->red = 1;
        q->link[0]->red = 0;
        q->link[1]->red = 0;
      }
      /* Fix red violation */
      if ( is_red ( q ) && is_red ( p ) ) {
        int dir2 = t->link[1] == g;
        if ( q == p->link[last] )
          t->link[dir2] = single_rotate ( g, !last );
        else
          t->link[dir2] = double_rotate ( g, !last );
      }
      /* Stop if found */
      if ( q->arrayAddress == arrayAddress )
        break;
      last = dir;
      dir = q->arrayAddress < arrayAddress;
      /* Update helpers */
      if ( g != NULL )
        t = g;
      g = p, p = q;
      q = q->link[dir];
    }
    /* Update root */
    tree->root = head.link[1];
  }
  /* Make root black */
  tree->root->red = 0;
  return 1;
}
