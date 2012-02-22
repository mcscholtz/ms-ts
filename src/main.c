/* 	Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

*/
#include <p33FJ128GP802.h>
#include "tasks.h"
#include "mempool.h"
#include "taskQueue.h"
#include "scheduler.h"
#include "init.h"
//#include "pipe.h"
/* NULL MACRO */
#define NULL (void *)0


// Select Internal FRC at POR
_FOSCSEL(FNOSC_FRC);
// Enable Clock Switching and Configure
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF)

void __attribute__((interrupt,no_auto_psv)) _T1Interrupt(void);

/* This is the System Tic timer interupt */
void __attribute__((interrupt)) _T1Interrupt()
{		
	IFS0bits.T1IF = 0;
	
	/* Update blocking tasks here */
	updateBlocking();	
}

int main() 
{
	/** Initialize Clock Settings **/
	
	// Configure PLL prescaler, PLL postscaler, PLL divisor
	PLLFBD = 41; // M = 43
	CLKDIVbits.PLLPOST=0; // N2 = 2
	CLKDIVbits.PLLPRE=0; // N1 = 2
	// Initiate Clock Switch to Internal FRC with PLL (NOSC = 0b001)
	__builtin_write_OSCCONH(0x01);
	__builtin_write_OSCCONL(0x01);
	// Wait for Clock switch to occur
	while (OSCCONbits.COSC != 0b001);
	// Wait for PLL to lock
	while(OSCCONbits.LOCK != 1) {}
	
	/* Enable nested interupts */
	INTCON1bits.NSTDIS = 0;
	
	/** Initialize IO Ports **/
	ADPCFG = 0xFFFF;
	TRISA = 0xFF;
	PORTA = 0x00;
	TRISB = 0x0000;
	PORTB = 0x0000;	
	
	/** Initialize core OS components **/
	initMemory();
	initTaskQueue();
	
	/** Create Tasks **/	
    createTask(&task1,1);
    createTask(&task2,2);
    createTask(&task3,3);
	createTask(&task4,4);
	createTask(&task5,5);
	createTask(&task6,6);
	createTask(&task7,7);
	createTask(&task8,8);
	
	/** Start the Scheduler **/
	startScheduler();
	
	//Will never get to this point unless there is a serious problem
	return 0;
}

