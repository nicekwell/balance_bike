#ifndef _UART_H_
#define _UART_H_
#include <stm32f10x_lib.h>

void uart2_init(void);
void uart2_sendB(u8 dat);
void uart2_sendStr(u8 *p);

#endif
