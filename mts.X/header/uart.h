#ifndef _UART_H___
#define _UART_H___


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
