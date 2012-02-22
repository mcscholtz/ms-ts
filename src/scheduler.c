/* 	Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

*/
#include <p33FJ128GP802.h>
#include "tasks.h"
#include "mempool.h"
#include "taskQueue.h"
#include "init.h"
/* NULL Macro */
#define NULL (void *)0

#define EXIT 0
#define YIELD 1
#define SLEEP 2

void getNextTask();

void updateBlocking();

extern struct queue *execTask;
extern struct queue *idleTask;

extern struct queue *rdyQueue_Tail;
extern struct queue *rdyQueue_Head;

extern struct queue *blockQueue_Tail;
extern struct queue *blockQueue_Head;

extern struct queue *taskMap[];

unsigned int stackPointer;

/* Pulls the first task ready to run and pushed it into the execute state */
/* Then execute the task */
void startScheduler()
{	
	/* Create an Idle Task */
	createTask(&idle,0);
	
	/* Start First Ready Task */
	getNextTask();
	
	/** Start the system timer **/
	init_TMR1();
	
	return execTask->TaskPtr();
	
}

int delay(long time)
{
	return  (int)(time/4);
}

/* FIFO scheduler */
/* There is no priority, tasks get scheduled as they are added to the readyQueue */
/* If there are no tasks in the ready queue an idle task gets scheduled to run until a task is ready to run again. */

void yield(int arg,int delay)
{
	// Disable Interupts
	SET_CPU_IPL(7); 
	switch(arg){
		case YIELD:
			//save the stack
			asm volatile("PUSH		SR		\n"
					"PUSH.D		W0			\n"
					"PUSH.D		W2			\n"
					"PUSH.D		W4			\n"
					"PUSH.D   	W6			\n"
					"PUSH.D   	W8			\n"
					"PUSH.D   	W10			\n"
					"PUSH.D		W12			\n"
					"PUSH		W14			\n"
					"PUSH		RCOUNT		\n"
					"PUSH		TBLPAG		\n"
					"PUSH		ACCAL		\n"
					"PUSH		ACCAH		\n"
					"PUSH		ACCAU		\n"
					"PUSH		ACCBL		\n"
					"PUSH		ACCBH		\n"
					"PUSH		ACCBU		\n"
					"PUSH		DCOUNT		\n"
					"PUSH		DOSTARTL	\n"
					"PUSH		DOSTARTH	\n"
					"PUSH		DOENDL		\n"
					"PUSH		DOENDH		\n"
					"PUSH		CORCON		\n"
					"PUSH		PSVPAG		\n"
					"MOV	    W15, _stackPointer	\n");
			execTask->sp = stackPointer;

			// Save task to ready queue
			if(execTask->id != 0){
				if(rdyQueue_Head){
					if(rdyQueue_Head->Next){
						rdyQueue_Tail->Next = execTask;
						execTask->Prev = rdyQueue_Tail;
						rdyQueue_Tail = execTask;	
					}else{
						rdyQueue_Tail = execTask;
						rdyQueue_Tail->Next = NULL;
						rdyQueue_Tail->Prev = rdyQueue_Head;
						rdyQueue_Head->Next = rdyQueue_Tail;	
					}
				}else{
						rdyQueue_Head = execTask;
						rdyQueue_Tail = execTask;
						idleTask = execTask;	
				}
			}else{
				//save idle task
				idleTask->sp = execTask->sp;
			}
			execTask = NULL;
			break;
		case SLEEP:
			//save the stack
			asm volatile("PUSH		SR		\n"
					"PUSH.D		W0			\n"
					"PUSH.D		W2			\n"
					"PUSH.D		W4			\n"
					"PUSH.D   	W6			\n"
					"PUSH.D   	W8			\n"
					"PUSH.D   	W10			\n"
					"PUSH.D		W12			\n"
					"PUSH		W14			\n"
					"PUSH		RCOUNT		\n"
					"PUSH		TBLPAG		\n"
					"PUSH		ACCAL		\n"
					"PUSH		ACCAH		\n"
					"PUSH		ACCAU		\n"
					"PUSH		ACCBL		\n"
					"PUSH		ACCBH		\n"
					"PUSH		ACCBU		\n"
					"PUSH		DCOUNT		\n"
					"PUSH		DOSTARTL	\n"
					"PUSH		DOSTARTH	\n"
					"PUSH		DOENDL		\n"
					"PUSH		DOENDH		\n"
					"PUSH		CORCON		\n"
					"PUSH		PSVPAG		\n"
					"MOV	    W15, _stackPointer	\n");	
			execTask->sp = stackPointer;

			execTask->counter = delay; //time in millioseconds	
		
			if(blockQueue_Head != NULL){
				execTask->Next = NULL;
				if(blockQueue_Head->Next){ //Case 1: there is more than 1 item in the queue
					execTask->Prev = blockQueue_Tail;			
					blockQueue_Tail->Next = execTask;
					blockQueue_Tail = execTask;	
				}else{ //Case 2: there is exactly 1 item in the queue
					blockQueue_Tail = execTask;
					blockQueue_Tail->Prev = blockQueue_Head;
					blockQueue_Head->Next = blockQueue_Tail;
				}		
			}else{ //Case 3: there are no items in the queue
				execTask->Next = NULL;
				execTask->Prev = NULL;
				blockQueue_Head = execTask;
				blockQueue_Tail = execTask;
			}
			execTask = NULL;
			break;
		case EXIT:
			//free task control block's memory
			taskMap[execTask->id] = NULL;
			freeBlock((int*)execTask); 
			execTask = NULL;
			break;
	}
	// Restore Context
	getNextTask();
	if(stackPointer != 0x0000){
		asm volatile("MOV		_stackPointer, W15	\n"
			"POP		PSVPAG				\n"
			"POP		CORCON				\n"
			"POP		DOENDH				\n"
			"POP		DOENDL				\n"
			"POP		DOSTARTH			\n"
			"POP		DOSTARTL			\n"
			"POP		DCOUNT				\n"
			"POP		ACCBU				\n"
			"POP		ACCBH				\n"
			"POP		ACCBL				\n"
			"POP		ACCAU				\n"
			"POP		ACCAH				\n"
			"POP		ACCAL				\n"
			"POP		TBLPAG				\n"
			"POP		RCOUNT				\n"		
			"POP		W14					\n"
			"POP.D		W12					\n"
			"POP.D		W10					\n"
			"POP.D		W8					\n"
			"POP.D		W6					\n"
			"POP.D		W4					\n"
		    "POP.D		W2					\n"
		    "POP.D		W0					\n"
			"POP		SR					\n");
	//re-enable Interupts
	SET_CPU_IPL(0); 
	}else{
		//re-enable Interupts
		SET_CPU_IPL(0); 
		// Task Runs for the first time
		execTask->TaskPtr();	
	}
	
}

void getNextTask()
{
	/* Get the next task that is ready to run */
	if(rdyQueue_Head){
		execTask = rdyQueue_Head;
		if(rdyQueue_Head->Next){
			rdyQueue_Head = rdyQueue_Head->Next;
			rdyQueue_Head->Prev = NULL;
		}else{
			rdyQueue_Head = NULL;
			rdyQueue_Tail = NULL;
		}
	}else{
		execTask = idleTask;
	}
	execTask->Next = NULL;
	
	/* Load the Ready task's stackpointer */
	stackPointer = execTask->sp;
}

/* Check all blocking tasks and update thier timers */
/* If any of the timers expire, add them to the ready queue */
void updateBlocking()
{

	struct queue *tmpNext = NULL;
	struct queue *tmpQueue = blockQueue_Head;		
	if(blockQueue_Head){	
		do{
			tmpQueue->counter--;
			if(tmpQueue->counter == 0){
				if(tmpQueue->Next){
					/* Case 1: First item on the blocking list */
					if(blockQueue_Head->id == tmpQueue->id){
						blockQueue_Head = blockQueue_Head->Next;
						blockQueue_Head->Prev = NULL;
					/* Case 2 : This item has another item infront of it and behind it */
					}else{	
						tmpQueue->Prev->Next = tmpQueue->Next;
						tmpQueue->Next->Prev = tmpQueue->Prev;
					}
				/* Case 3: This is the last item on the blocking queue */ 	
				}else{
					if(tmpQueue->Prev){
						blockQueue_Tail = blockQueue_Tail->Prev;
						blockQueue_Tail->Next = NULL;
						if(blockQueue_Tail == blockQueue_Head){
							blockQueue_Head->Next = NULL;
						}
					}else{
						blockQueue_Tail = NULL;
						blockQueue_Head = NULL;	
					}
				}
					/* Save Next pointer temporarily */
					tmpNext = tmpQueue->Next;
					/* Add task back to ready queue (at the back of queue) */
					if(rdyQueue_Head){					
						if(rdyQueue_Head->Next){//Case 1: More than 1 item in queue
							tmpQueue->Prev = rdyQueue_Tail;
							rdyQueue_Tail->Next = tmpQueue;
							rdyQueue_Tail = tmpQueue;
						}else{ //Case 2: Only 1 item in queue
							tmpQueue->Prev = rdyQueue_Head;
							rdyQueue_Head->Next = tmpQueue;
							rdyQueue_Tail = tmpQueue;	
						}
						rdyQueue_Tail->Next = NULL;
					}else{ //Case 2: No items in ready queue yet
						rdyQueue_Head = tmpQueue;	
						rdyQueue_Head->Next = NULL;	
						rdyQueue_Head->Prev = NULL;
						rdyQueue_Tail = rdyQueue_Head;	 
					}
				}
			if(tmpNext){
				tmpQueue = tmpNext;
				tmpNext = NULL;
			}else{
				tmpQueue = tmpQueue->Next;	
			}
		}while(tmpQueue);
	}
}
