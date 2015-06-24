#include <p33FJ128GP802.h>
#include "tasks.h"
#include "scheduler.h"
#include "taskQueue.h"
#include "init.h"
#include "uart.h"
#include "config.h"

#define NULL (void *)0


// Select Internal FRC at POR
_FOSCSEL(FNOSC_FRC);
// Enable Clock Switching and Configure Posc in XT mode
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF & POSCMD_XT);
//Turn off WatchDog Timer
_FWDT(FWDTEN_OFF);										
_FPOR(FPWRT_PWR1);
//Disable Code Protection									
_FGS(GSS_OFF);
//debug on channel 2 & disable JTAG
_FICD(ICS_PGD2 & JTAGEN_OFF)

//Define UART Tx DMA buffers
char TX_buff_A[UART_TX_BUFF_SIZE] __attribute__((space(dma)));
char TX_buff_B[UART_TX_BUFF_SIZE] __attribute__((space(dma)));

void __attribute__((interrupt,no_auto_psv)) _T1Interrupt(void);
void __attribute__((interrupt,no_auto_psv)) _DMA0Interrupt(void);
void __attribute__((interrupt,no_auto_psv)) _U1RXInterrupt();

/* This is the System Tic timer interupt */
void __attribute__((interrupt,no_auto_psv)) _T1Interrupt()
{		
    /* Update blocking tasks here */
    IFS0bits.T1IF = 0;
    updateBlocking();
}

/*
void __attribute__((__interrupt__,no_auto_psv)) _U1ErrInterrupt(){
	// An error has occured on the last reception. Check the last received word.
	IFS4bits.U1EIF = 0;
	int lastWord;
	// Check which DMA Ping pong channel was selected.
	if(DMACS1bits.PPST1 == 0){
	// Get the last word received from ping pong buffer A.
	//	lastWord = *(unsigned int *)((unsigned int)(BufferA) + DMA1STA);
	}else{
	// Get the last word received from ping pong buffer B.
	//	lastWord = *(unsigned int *)((unsigned int)(BufferB) + DMA1STB);
	}
	// Check for Parity Error
	if((lastWord & 0x800) != 0){
	// There was a parity error, Do something about it here.
	}
	// Check for framing error
	if ((lastWord & 0x400) != 0){
	// There was a framing error, Do some thing about it here.
	}
}
*/

void __attribute__((interrupt,no_auto_psv)) _DMA0Interrupt()
{
    extern struct tx_buffer *tx_buff_a;
    extern struct tx_buffer *tx_buff_b;
    extern int active_tx_buffer;
    int i;
    DMA0CONbits.CHEN = 0;//disable the dma0 channel
    //set B as active
    if(active_tx_buffer){ //B is active
        for(i=0;i<tx_buff_b->buffer_ptr;i++){
            TX_buff_B[i] = 0;
        }
        tx_buff_b->buffer_ptr = 0;
	if(tx_buff_a->queued){
            //start A
            DMA0CNT = tx_buff_a->buffer_ptr-1;
            DMA0STA = __builtin_dmaoffset(TX_buff_A);
            DMA0CONbits.CHEN = 1;// Enable DMA0 Channel
            DMA0REQbits.FORCE = 1;
            //set A as active
            active_tx_buffer = 0;
            tx_buff_a->queued = 0;
         }
    }else{ 
        for(i=0;i<tx_buff_a->buffer_ptr;i++){
            TX_buff_A[i] = 0;
        }
        tx_buff_a->buffer_ptr = 0;
	if(tx_buff_b->queued){
            //start A
            DMA0CNT = tx_buff_b->buffer_ptr-1;
            DMA0STA = __builtin_dmaoffset(TX_buff_B);
            DMA0CONbits.CHEN = 1;// Enable DMA0 Channel
            DMA0REQbits.FORCE = 1;
            //set A as active
            active_tx_buffer = 1;
            tx_buff_b->queued = 0;
	}
    }
    IFS0bits.DMA0IF = 0;// Clear the DMA0 Interrupt Flag;
}

void __attribute__((interrupt,no_auto_psv)) _U1RXInterrupt()
{
    IFS0bits.U1RXIF = 0; // clear TX interrupt flag
    //RX buffer contains data
    if(U1STAbits.URXDA){
        //start polling
        createTask(&poll_UART1_RX,1);
        LATBbits.LATB1 = ~LATBbits.LATB1;
        //disable this interupt
        IEC0bits.U1RXIE = 0;
    }
}

int main() 
{
    /** Initialize Clock Settings **/
    // Configure PLL prescaler, PLL postscaler, PLL divisor 73.728MHz
    PLLFBD=80; // M = 32
    CLKDIVbits.PLLPOST = 0;// N2 = 2
    CLKDIVbits.PLLPRE = 0; // N1 = 2
    // Initiate Clock Switch to Primary Oscillator with PLL (NOSC = 0b011)
    __builtin_write_OSCCONH(0x03);
    __builtin_write_OSCCONL(0x01);
    // Wait for Clock switch to occur
    while (OSCCONbits.COSC != 0b011);
    // Wait for PLL to lock
    while(OSCCONbits.LOCK != 1) {};
	
    /* Enable nested interupts */
    INTCON1bits.NSTDIS = 0;
	
    /*Init UART Tx DMA */
    DMA0CON = 0x6001; // One-Shot, Post-Increment, RAM-to-Peripheral
    DMA0REQ = 0x000C; // Select UART1 Transmitter
    DMA0PAD = (volatile unsigned int) &U1TXREG;
    IFS0bits.DMA0IF = 0; // Clear DMA Interrupt Flag
    IEC0bits.DMA0IE = 1; // Enable DMA Interrupt
	
    /** Initialize UART Port **/
    ADPCFG = 0xFFFF;
    //TRISA = 0x02;
    //PORTA = 0x00;
    TRISB = 0xA080;
    PORTB = 0x0000;
    //Enable Remapping
    asm volatile (  "mov #OSCCONL, w1  \n"
                    "mov #0x46, w2     \n"
                    "mov #0x57, w3     \n"
                    "mov.b w2, [w1]    \n"
                    "mov.b w3, [w1]    \n"
                    "bclr OSCCON, #6");
    
    //Remap peripherals	
        RPINR18bits.U1RXR = 7;          //uart1 Rx -> rp7
        RPOR3bits.RP6R = 0b00011;       //uart1 Tx -> rp6
    #ifdef FLOW_CONTROL
        RPINR18bits.U1CTSR = 8;         //uart1 cts -> rp8
        RPOR4bits.RP9R = 0b00100;       //uart1 rts -> rp9
    #endif

    //Disable Remapping
    asm volatile (  "mov #OSCCONL, w1  \n"
                    "mov #0x46, w2     \n"
                    "mov #0x57, w3     \n"
                    "mov.b w2, [w1]    \n"
                    "mov.b w3, [w1]    \n"
                    "bset OSCCON, #6");
				  
    InitUART1();
	
    /** Initialize core OS components **/
    initTaskQueue();
	
	
    /** Create Tasks **/
    createTask(&parse_input,2);
//    createTask(&task3,3);
//    createTask(&task4,4);
//    createTask(&task5,5);
//    createTask(&task6,6);
//    createTask(&task7,7);
//    createTask(&task8,8);

    /** Start the Scheduler **/
    startScheduler();

    //Will never get to this point unless there is a serious problem
    return 0;
}
