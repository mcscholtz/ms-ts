/* 	Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

*/
#ifndef _INIT_H___
#define _INIT_H___

#define SYS_TICK 0x2710
//0x2710 = ~1ms timer overflow if clock @ 80Mhz
void init_TMR1();
void init_TMR2();
#endif
