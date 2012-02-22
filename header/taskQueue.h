/* 	Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

*/
#ifndef _TASKQUEUE_H___
#define _TASKQUEUE_H___

#define MAX_TASKS 8

/** Pipe Control block **/
/*
struct pcb {	
	int is_read;		//a flag used to check if the data has been read or not
	int *data;
};
*/
/** Used for the Ready Queue **/
struct queue {
	void (*TaskPtr)();
	int id	:	8;			/* unique task identifier */
	int counter;			/* used when a task is blocking to delay */
	int sp;					/* contain the task's stackpointer */
	struct queue *Next; 	/* points to the next member of the queue */
	struct queue *Prev; 	/* points to the previous queue member */
};

void createTask(void (*Ptr2Func)(), int id);
void initTaskQueue();

#endif
