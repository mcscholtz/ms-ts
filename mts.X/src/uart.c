#include <p33FJ128GP802.h>
#include <stdlib.h>
#include <string.h>
#include "uart.h"
#include "config.h"
#include <stdbool.h>


#define CHAN_A 0
#define CHAN_B 1

int fill_dma_buffer_s(int buffer,char* data);
int fill_dma_buffer_c(int buffer,char data);

extern char TX_buff_A[UART_TX_BUFF_SIZE] __attribute__((space(dma)));
extern char TX_buff_B[UART_TX_BUFF_SIZE] __attribute__((space(dma)));;
char RX_buff[UART_RX_BUFF_SIZE];

struct tx_buffer *tx_buff_a = NULL;
struct tx_buffer *tx_buff_b = NULL;
struct rx_buffer *rx_buff = NULL;

int active_tx_buffer; //0 = a, 1 = b

void InitUART1() {

	U1MODEbits.UARTEN = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
	U1MODEbits.USIDL = 0;	// Bit13 Continue in Idle
	U1MODEbits.IREN = 0;	// Bit12 No IR translation
        #ifdef FLOW_CONTROL
        //FlowControl Mode
        U1MODEbits.RTSMD = 0;
        //TX,RX,CTS & RTS enabled
        U1MODEbits.UEN = 2;
        #else
        //Simplex mode
        U1MODEbits.RTSMD = 1;
        //TX & RX enabled, CTS & RTS not enabled
        U1MODEbits.UEN = 0;
        #endif
	U1MODEbits.WAKE = 0;	// Bit7 No Wake up (since we don't sleep here)
	U1MODEbits.LPBACK = 0;	// Bit6 No Loop Back
	U1MODEbits.ABAUD = 0;	// Bit5 No Autobaud (would require sending '55')
	U1MODEbits.URXINV = 0;	// Bit4 IdleState = 1  (for dsPIC)
	U1MODEbits.BRGH = 0;	// Bit3 16 clocks per bit period
//	U1MODEbits.BRGH = 1;	// Bit3 4 clocks per bit period
	U1MODEbits.PDSEL = 0;	// Bits1,2 8bit, No Parity
//	U1MODEbits.PDSEL = 1;	// Bits1,2 8bit, even Parity
	U1MODEbits.STSEL = 0;	// Bit0 One Stop Bit
	
	//  U1BRG = (Fcy / (16 * BaudRate)) - 1
	//  U1BRG = (36864000 / (16 * 9600)) - 1
	U1BRG = 239;

       // U1BRG = ((CLOCK / (16 * BAUD_RATE)) - 1);
	
//	U1BRG = 239; //9600 ~1 char per 1.04 msec
//	U1BRG = 119; //19200 ~1 char per 528 usec
//	U1BRG = 59; //38400 ~1 char per 260.4 usec
//	U1BRG = 19; //115200 ~1char per 86.8 usec
//	U1BRG = 9; //921600 ~1char per 10.9 usec
	
	//bit 15,13 = interupt when data is pushed out of the uart
	// Load all values in for U1STA SFR
	U1STAbits.UTXISEL1 = 0;	//Bit15 Int when Char is transferred (1/2 config!)
	U1STAbits.UTXINV = 0;	//Bit14 N/A, IRDA config
	U1STAbits.UTXISEL0 = 0;	//Bit13 int when all transfer is complete
	U1STAbits.UTXBRK = 0;	//Bit11 Disabled
	U1STAbits.UTXEN = 0;	//Bit10 TX pins controlled by periph
	U1STAbits.URXISEL = 0;	//Bits6,7 Int. on character recieved
	U1STAbits.ADDEN = 0;	//Bit5 Address Detect Disabled

	U1MODEbits.UARTEN = 1;	// And turn the peripheral on
	U1STAbits.UTXEN = 1;	
	//DMA TX Buffers
	tx_buff_a = malloc(sizeof(struct tx_buffer));
	tx_buff_b = malloc(sizeof(struct tx_buffer));	
	tx_buff_a->buffer_ptr = 0;	
	tx_buff_b->buffer_ptr = 0;
        tx_buff_a->queued = FALSE;
        tx_buff_b->queued = FALSE;
	active_tx_buffer = CHAN_B; //B starts off as the active buffer
	
	//RX Buffers
	rx_buff = malloc(sizeof(struct rx_buffer));
	rx_buff->write_ptr = 0;
	rx_buff->read_ptr = 0;
	rx_buff->is_full = FALSE;
	IEC0bits.U1RXIE = 1;    //start with RX interupt enabled

	write_uart_s("Loading...");
	commit_uart();
}

int write_uart_s(char* string)
{
    if(active_tx_buffer){ //b is active
	//write to A
	if(!tx_buff_a->queued){
            if(!fill_dma_buffer_s(CHAN_A,string)){
                //error there was no room, handle it here
		return ERROR;
            }
	}else{
            return ERROR;
	}
    }else{
	//write to B
        if(!tx_buff_b->queued){
            if(!fill_dma_buffer_s(CHAN_B,string)){
                //error there was no room, handle it here
		return ERROR;
            }
	}else{
            return ERROR;
	}
    }
    return SUCCESS;
}

int write_uart_c(char character)
{
	if(active_tx_buffer){ //b is active
		//write to A
		if(!tx_buff_a->queued){
			if(!fill_dma_buffer_c(CHAN_A,character)){
				//error there was no room, handle it here
				return ERROR;
			}
		}else{
			return ERROR;
		}
	}else{
		//write to B
		if(!tx_buff_b->queued){
			if(!fill_dma_buffer_c(CHAN_B,character)){
				//error there was no room, handle it here
				return ERROR;
			}
		}else{
			return ERROR;
		}
	}
	return SUCCESS;
}	

//write to DMA buffer specified
//incriment buffer pointer
int fill_dma_buffer_s(int buffer,char* data)
{
	int data_len = strlen(data);
	int i,j,total_len;
	if(buffer == CHAN_A){
		total_len = data_len + tx_buff_a->buffer_ptr;
		if(total_len < UART_TX_BUFF_SIZE){
			//there is enough room, write to dma buffer
			j=0;
			for(i=tx_buff_a->buffer_ptr;i<total_len;i++){
				TX_buff_A[i] = data[j];
				j++;
			}
			tx_buff_a->buffer_ptr += data_len;
			return SUCCESS;
		}else{
			//no room, return failure
			return ERROR;
		}
	}else if(buffer == CHAN_B){
		//write to buffer B
		total_len = data_len + tx_buff_b->buffer_ptr;
		if(total_len < UART_TX_BUFF_SIZE){
			//there is enough room, write to dma buffer
			j=0;
			for(i=tx_buff_b->buffer_ptr;i<total_len;i++){
				TX_buff_B[i] = data[j];
				j++;
			}
			tx_buff_b->buffer_ptr += data_len;
				return SUCCESS;
		}else{
			//no room, return failure
			return ERROR;
		}
	}else{
		return ERROR;
	}
}

int fill_dma_buffer_c(int buffer,char data)
{
	if(buffer == CHAN_A){
		if((1 + tx_buff_a->buffer_ptr) <= UART_TX_BUFF_SIZE){
			//there is enough room, write to dma buffer
			TX_buff_A[tx_buff_a->buffer_ptr] = data;
			tx_buff_a->buffer_ptr++;
			return SUCCESS;
		}else{
			//no room, return failure
			return ERROR;
		}
	}else if(buffer == CHAN_B){
		//write to buffer B
		if((1 + tx_buff_b->buffer_ptr) <= UART_TX_BUFF_SIZE){
			//there is enough room, write to dma buffer
			TX_buff_B[tx_buff_b->buffer_ptr] = data;
			tx_buff_b->buffer_ptr++;
			return SUCCESS;
		}else{
			//no room, return failure
			return ERROR;
		}
	}else{
		return ERROR;
	}
}

int get_tx_buffer_space()
{
	if(active_tx_buffer == 0){
		return (UART_TX_BUFF_SIZE - tx_buff_a->buffer_ptr);
	}else{
		return (UART_TX_BUFF_SIZE - tx_buff_b->buffer_ptr);
	}
}

void commit_uart()
{
	if(active_tx_buffer){ //b active
		//queue up A
		if(DMA0CONbits.CHEN){			
			tx_buff_a->queued = 1;
		}else{
			//start A
			DMA0CNT = tx_buff_a->buffer_ptr-1;
			DMA0STA = __builtin_dmaoffset(TX_buff_A);
			DMA0CONbits.CHEN = 1;// Enable DMA0 Channel
			DMA0REQbits.FORCE = 1;
			//set A as active
			active_tx_buffer = CHAN_A;			
		}
	}else{
		//queue up b
		if(DMA0CONbits.CHEN){
			tx_buff_b->queued = 1;
		}else{
			//start B
			DMA0CNT = tx_buff_b->buffer_ptr-1;
			DMA0STA = __builtin_dmaoffset(TX_buff_B);
			DMA0CONbits.CHEN = 1;// Enable DMA0 Channel
			DMA0REQbits.FORCE = 1;
			//set B as active
			active_tx_buffer = CHAN_B;
		}
	}
}

int push_rx_data(char data)
{
	if(!rx_buff->is_full){
		RX_buff[rx_buff->write_ptr] = data;
	}else{
		return ERROR;
	}
	//Next write still needs to be read, flag as full
	if(rx_buff->write_ptr+1 == rx_buff->read_ptr){
		rx_buff->write_ptr++;
		rx_buff->is_full = TRUE;
	}else if(rx_buff->write_ptr+1 == UART_RX_BUFF_SIZE){
		//Next write still needs to be read, flag as full
		if(rx_buff->read_ptr == 0){
			rx_buff->write_ptr = 0;
			rx_buff->is_full = TRUE;
		}else{
			rx_buff->write_ptr = 0;
		}
	}else{
		rx_buff->write_ptr++;
	}
	return SUCCESS;
}

char get_rx_data()
{
	//if rx_buff->write_ptr == rx_buff->read_ptr then there is nothing to read
	if(rx_buff->write_ptr != rx_buff->read_ptr){
		char data = RX_buff[rx_buff->read_ptr];
		if(rx_buff->read_ptr+1 == UART_RX_BUFF_SIZE){
			rx_buff->read_ptr = 0;
		}else{
			rx_buff->read_ptr++;
		}
		if(rx_buff->is_full){
			rx_buff->is_full = FALSE;
		}
		return data;
	}else{
		return ERROR;
	}
}

int check_rx_buff()
{
	if(rx_buff->write_ptr != rx_buff->read_ptr){
		return 1;
	}else{
		return 0;
	}
}
