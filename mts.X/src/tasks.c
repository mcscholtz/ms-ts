#include <p33FJ128GP802.h>
#include <string.h>
#include <stdlib.h>
#include "scheduler.h"
#include "taskQueue.h"
#include "uart.h"

void task7();
void parse_input();
/* Task Shared variables */

/* This is the idle task, it will be run whenever there are no other tasks to run */
void idle()
{
    extern struct queue *rdyQueue_Head;
    for(;;)
    {
        if(rdyQueue_Head){
            yield(YIELD,0);
	}
    }
}

/* This task is automattically started when there is data detected on the serial port */

/* Need to implement a method of killing this task if there is no more data and re-enabling
 * the uart interupt that starts this task in the event that there is more data detected */
void poll_UART1_RX()
{
    char data;
    int timeout=0;
    for(;;)
    {
        LATBbits.LATB0 = ~LATBbits.LATB0; //Polling
        //If there is no UART activity stop polling
        if(!U1STAbits.URXDA){
            timeout++;
            if(timeout >= RX_POLL_TIMEOUT){
                LATBbits.LATB0 = 0; //not polling
                IFS0bits.U1RXIF = 0;
                IEC0bits.U1RXIE = 1;
                yield(EXIT,0);
            }      
        }else{
            //While there is data to read, read the data
            while(U1STAbits.URXDA){
                data = U1RXREG;
                if(!push_rx_data(data)){
                    //characters are being droped
                    LATBbits.LATB2 = 1;
                }else{
                    //there are no problems
                    LATBbits.LATB2 = 0;
                }
            }
        }
	yield(YIELD,0);
    }
}


void parse_input()
{
    char string_buffer[10],data;
    int i=0,j=0;//,res=0;
    char *strptr;
    strptr = (char *)string_buffer;   
    for(;;)
    {
        while(check_rx_buff()){ //there is something in read fifo
            data = get_rx_data();
            if(data != 0xD && data != 0xA){
                *strptr = data;
                strptr++;
                i++;
                if(i == 9){
                    i=0;
                    strptr = (char *)string_buffer;
                }
            }
            if(data == 0xD || data == 0xA){
                i=0;
                strptr = (char *)string_buffer;
            }
            //yield(SLEEP,100); //wait for the request
            if((strncmp(strptr,"GET",3) == 0)){
                yield(SLEEP,250); //wait for the whole request before replying
                write_uart_s("HTTP/1.0 200 OK\r\n");
                write_uart_s("Content-type: text/html\r\n");
                write_uart_s("Content-length: 109\r\n\r\n");
                write_uart_s("<HTML><BODY>");
                write_uart_s("<form action=");
                write_uart_c('"');
                write_uart_s("led.html");
                write_uart_c('"');
                write_uart_s(" method=");
                write_uart_c('"');
                write_uart_s("get");
                write_uart_c('"');
                write_uart_s(">");
                write_uart_s("<input type=");
                write_uart_c('"');
                write_uart_s("submit");
                write_uart_c('"');
                write_uart_s(" value=");
                write_uart_c('"');
                write_uart_s("Click me!");
                write_uart_c('"');
                write_uart_s("></form>");
                write_uart_s("</BODY></HTML>\r\n\r\n");
                commit_uart();
                yield(SLEEP,250); //wait for all data to send
                i=0;
                strptr = (char *)string_buffer;
                for(j=0;j<10;j++){
                    *strptr = 0;
                    strptr++;
                }
            }
        }
        yield(SLEEP,50);
    }
    yield(EXIT,0);
}


void task3()
{
    for(;;)
    {
        LATBbits.LATB0 = ~LATBbits.LATB0;
        createTask(&task7,7);
        yield(SLEEP,50);
    }
    yield(EXIT,0);
}

void task4()
{
    for(;;)
    {
        LATBbits.LATB1 = ~LATBbits.LATB1;
        yield(SLEEP,50);
    }
    yield(EXIT,0);
}

void task5()
{
    for(;;)
    {
        LATBbits.LATB2 = ~LATBbits.LATB2;
        yield(SLEEP,50);
    }
    yield(EXIT,0);
}

void task6()
{
    for(;;)
    {
        LATBbits.LATB3 = ~LATBbits.LATB3;
        yield(SLEEP,50);
    }
    yield(EXIT,0);
}

void task7()
{
    for(;;)
    {
        LATBbits.LATB4 = ~LATBbits.LATB4;
        yield(EXIT,0);
    }
    yield(EXIT,0);
}

void task8()
{
    for(;;)
    {
        LATBbits.LATB5 = ~LATBbits.LATB5;
        yield(YIELD,0);
    }
    yield(EXIT,0);
}

