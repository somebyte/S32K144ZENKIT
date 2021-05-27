/*
 * tty.h
 *
 *  Created on: Apr 20, 2021
 *      Author: somebyte
 */

#ifndef TTY_TTY_H_
#define TTY_TTY_H_

#define MAX_CANON  255 /* Maximum number of bytes in a terminal canonical */
                       /* input line.                                     */
#define MAX_INPUT  255 /* Minimum number of bytes for which space is      */
                       /* available in a terminal input queue; therefore, */
                       /* the maximum number of bytes a conforming        */
                       /* application may require to be typed as input    */
                       /* before reading them.                            */
#define MAX_NUMBF  25

#define NUL 0x00 /* NULL                      */
#define EOT 0x04 /* END OF TRANSMIT           */
#define ACK 0x06 /* ACKNOWLEDGMENT            */
#define ETB 0x17 /* END OF TRANSMISSION BLOCK */
#define CR  0x0D /* CARRIAGE RETURN           */
#define CAN 0x18 /* CANCEL                    */
#define ESC 0x1B /* ESCAPE                    */

#endif /* TTY_TTY_H_ */
