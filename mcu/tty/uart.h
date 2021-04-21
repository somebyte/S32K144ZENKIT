/* Author: somebyte */
/* 2019.11.11       */

#ifndef _TTY_UART_H_
#define _TTY_UART_H_

#include <stdint.h>

/* UART_IFC - uart interface */
#define UART_IFCMASK 0x0000000F
#define UART_IFC0    0x00000001
#define UART_IFC1    0x00000002
#define UART_IFC2    0x00000004

/* UART_PIN_RX | UART_PIN_TX */
/* LPUART0: RX0 -> PTA2;  RX1 -> PTB0; RX2 -> PTC2  */
/* LPUART1: RX0 -> PTC6;  RX1 -> PTC8; RX2 -> PTD13 */
/* LPUART2: RX0 -> PTA8;  RX1 -> PTD6; RX2 -> PTD17 */
#define UART_PIN_RXMASK 0x000000F0
#define UART_PIN_RX0    0x00000010
#define UART_PIN_RX1    0x00000020
#define UART_PIN_RX2    0x00000040
/* LPUART0: TX0 -> PTA3;  TX1 -> PTB1; TX2 -> PTC3  */
/* LPUART1: TX0 -> PTC7;  TX1 -> PTC9; TX2 -> PTD14 */
/* LPUART2: TX0 -> PTA9;  TX1 -> PTD7; TX2 -> PTE12 */
#define UART_PIN_TXMASK 0x00000F00
#define UART_PIN_TX0    0x00000100
#define UART_PIN_TX1    0x00000200
#define UART_PIN_TX2    0x00000400

/* UART_BAUD - uart speed */
#define UART_BMASK   0x00FF0000
#define UART_B9600   0x00010000
#define UART_B19200  0x00020000
#define UART_B38400  0x00040000
#define UART_B57600  0x00060000
#define UART_B115200 0x000C0000
#define UART_B230400 0x00180000
#define UART_B460800 0x00300000
#define UART_B921600 0x00600000

void uart_init  (uint32_t settings); /* settings = UART_IFC|UART_PIN_RX|UART_PIN_TX|UART_BAUD */
void uart_putc  (char  ch);
void uart_puts  (char* buf, uint32_t maxsize);
char uart_getc  (void);
int  uart_gets  (char* buf, uint32_t maxsize);
void uart_reset (void);

#endif /* _TTY_UART_H_ */

