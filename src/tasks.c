/* 	Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

*/
#include <p33FJ128GP802.h>
#include "scheduler.h"
#include "mempool.h"
#include "taskQueue.h"

#define NULL (void *)0
#define EXIT 0
#define YIELD 1
#define SLEEP 2
void idle();
void task1();
void task2();
void task3();
void task4();
void task5();
void task6();
void task7();
void task8();

/* Task Shared variables */

/* This is the idle task, it will be run whenever there are no other tasks to run */
void idle()
{
	extern struct queue *rdyQueue_Head;
    while(1)
    {
		if(rdyQueue_Head){
        	yield(YIELD,0);
		}
    }
}

void task1()
{
    while(1)
    {		
		PORTBbits.RB0 = ~PORTBbits.RB0;
		yield(SLEEP,delay(250)); 		
    }
}

void task2()
{
	while(1)
    {	
		PORTBbits.RB1 = ~PORTBbits.RB1;
		yield(SLEEP,delay(25));	
    }
}

void task3()
{	
	while(1)
    {
		yield(SLEEP,delay(15));
    }
}

void task4()
{
	while(1)
	{
		yield(SLEEP,delay(15));
	}
}

void task5()
{	
	while(1)
    {
		yield(SLEEP,delay(15));
    }
}

void task6()
{	
	while(1)
    {
		yield(SLEEP,delay(15));
    }
}

void task7()
{	
	while(1)
    {
		yield(SLEEP,delay(15));
    }
}

void task8()
{
	while(1)
    {
		yield(SLEEP,delay(15));
    }   
}
