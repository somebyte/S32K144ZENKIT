#ifndef __TTYSPEED_H__
#define __TTYSPEED_H__

#include <termios.h>
#include <stdio.h>

#define NUL 0x00 /* NULL                      */
#define EOT 0x04 /* END OF TRANSMIT           */
#define ACK 0x06 /* ACKNOWLEDGMENT            */
#define ETB 0x17 /* END OF TRANSMISSION BLOCK */
#define CR  0x0D /* CARRIAGE RETURN           */
#define CAN 0x18 /* CANCEL                    */
#define ESC 0x1B /* ESCAPE                    */

speed_t baudrate(const char* from);

FILE* open_tty (const char* filepath, speed_t baudrate);

#endif

