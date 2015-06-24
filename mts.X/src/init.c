/* 	Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

*/
#include <p33FJ128GP802.h>
#include "init.h"
#include "config.h"

void init_TMR1( void )
{
	PMD1bits.T1MD =0;               // TMR1 is enabled
	T1CON = 0;                      // ensure Timer 1 is in reset state	
	IEC0bits.T1IE = 1;              // enable Timer 1 interrupt
 	IFS0bits.T1IF = 0;              // reset Timer 1 interrupt flag 	
	IPC0bits.T1IP = 6;			
	PR1 = ((SYS_CLOCK/SYS_TICK));
    T1CONbits.TCKPS = 0;		// prescaler set to 1:256
 	T1CONbits.TCS = 0;              // select internal timer clock
	T1CONbits.TSIDL = 1;            // continue when cpu idle
	TMR1 = 0x0000;
	T1CONbits.TON = 1;              // enable Timer 1 and start the count		
}
