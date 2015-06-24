#ifndef _UART_H___
#define _UART_H___

#define TX_BUFF_SIZE 256
#define RX_BUFF_SIZE 128

//The number of cycles of inactivity before the UART polling is shutdown
#define RX_POLL_TIMEOUT 256

#define CLOCK 36864000
#define BAUD_RATE 9600//38400
//#define FLOW_CONTROL  //comment this line out if you dont use flow control

struct tx_buffer {
	short queued;
	unsigned int buffer_ptr;
};
struct rx_buffer {
	unsigned int write_ptr;
	unsigned int read_ptr;
	short is_full;
};

void InitUART1();
int write_uart_s(char* string);
int write_uart_c(char character);
void commit_uart();
int get_tx_buffer_space();
int push_rx_data(char data);
int check_rx_buff();
char get_rx_data();

#endif
