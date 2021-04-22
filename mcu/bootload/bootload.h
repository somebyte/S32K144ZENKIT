/*
 * bootload.h
 *
 *  Created on: Apr 21, 2021
 *      Author: somebyte
 */

#ifndef BOOTLOAD_BOOTLOAD_H_
#define BOOTLOAD_BOOTLOAD_H_

#include "../tty/uart.h"

#define APPDEBUG /* If you debug application through openSDA,  */
#undef  APPDEBUG /* FIRC have to work. Comment this for debug. */

extern int need_jump_to_fw;

int bootloadmain (uint32_t uartcfg);

int upload (const void*);
int jump   (const void*);

#endif /* BOOTLOAD_BOOTLOAD_H_ */
