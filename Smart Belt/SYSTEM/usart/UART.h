#ifndef _UART_H
#define _UART_H

//void UART1_Init(void);
//void UART2_Init(void);
void UART1_SendByte(uint8_t Byte);
void UART2_SendByte(uint8_t Byte);
void Serial_SendString(char Strings[],FlagStatus LineBreak);
void Serial_SendMessage(char *fmt,...);

#endif
