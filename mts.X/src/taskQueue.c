/* 	Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

*/
#include <p33FJ128GP802.h>
#include <stdlib.h>
#include "tasks.h"
#include "taskQueue.h"

/* The +1 is to account for the idle task */
struct queue *taskMap[MAX_TASKS+1];

struct queue *execTask = NULL;
struct queue *idleTask = NULL;

struct queue *rdyQueue_Tail = NULL;
struct queue *rdyQueue_Head = NULL;

struct queue *blockQueue_Head = NULL;
struct queue *blockQueue_Tail = NULL;

struct queue *sQueue_Head = NULL;
struct queue *sQueue_Tail = NULL;

/* Initialize task Map to NULL */
void initTaskQueue()
{
	int i;
	for(i=0;i<(MAX_TASKS+1);i++){
		taskMap[i] = NULL;
	}
}

/* Creates a Task and add it to the end of the ready queue */
//Id's should eventually be auto assigned by the OS
void createTask(void (*Ptr2Func)(), int id/*, int priority*/)
{
	/* Only add a new task if its ID is unique */
	if(!taskMap[id]){
		struct queue *tmpQueue = NULL;
		//tmpQueue = (struct queue *)allocBlock(sizeof(struct queue));
                tmpQueue = malloc(sizeof(struct queue));
		tmpQueue->TaskPtr = Ptr2Func;
		tmpQueue->id = id;
    //            tmpQueue->priority = priority;
    //            tmpQueue->eff_priority = priority;
		tmpQueue->sp = 0;
		tmpQueue->counter = 0;
		tmpQueue->Next = NULL;
		taskMap[id] = tmpQueue;

		if(Ptr2Func == &idle){
			idleTask = tmpQueue;
			tmpQueue->Prev = NULL;
		}else if(rdyQueue_Head){
			tmpQueue->Prev = rdyQueue_Tail;
			rdyQueue_Tail->Next = tmpQueue;
			rdyQueue_Tail = tmpQueue;
		}else{
			rdyQueue_Tail = tmpQueue;
			rdyQueue_Head = rdyQueue_Tail;
			tmpQueue->Prev = NULL;
		}
	}
}
