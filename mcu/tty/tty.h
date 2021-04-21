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

#define EOT 0x04 /* End of Transmit */

#endif /* TTY_TTY_H_ */
