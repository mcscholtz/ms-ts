/* 	Written by Marius Scholtz 2012
	
	This code is provides as is, and may be used and modified as you like. However please give credit where credit is due

*/
#ifndef _SCHEDULER_H___
#define _SCHEDULER_H___

int delay(long time);
void yield(int arg,int delay);
void startScheduler();
void updateBlocking();

#endif
