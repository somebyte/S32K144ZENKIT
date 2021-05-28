/*
 * bootload.h
 *
 *  Created on: Apr 21, 2021
 *      Author: somebyte
 */

#ifndef BOOTLOAD_BOOTLOAD_H_
#define BOOTLOAD_BOOTLOAD_H_

#include "../tty/uart.h"
#include "../fw/fw.h"

typedef enum
{
  BOOTL_RESETED = 0,
  BOOTL_INITED  = 1,
  BOOTL_JUMP    = 2
} bootload_t;

bootload_t bootloadinit  (uint32_t uartcfg);
bootload_t bootloadmain  ();
bootload_t bootloadreset ();

#define BOOTLOAD(UARTCFG) {\
    STARTBOOL:\
      while(1)\
        {\
          bootloadinit (UARTCFG);\
          if (bootloadmain  () == BOOTL_JUMP)\
    	    {\
    	      bootloadreset ();\
    	      break;\
    	    }\
        }\
      jump_to_fw ();\
    goto STARTBOOL; }

#endif /* BOOTLOAD_BOOTLOAD_H_ */
