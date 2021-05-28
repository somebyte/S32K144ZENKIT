/*
 * evbtty.h
 *
 *  Created on: May 18, 2021
 *      Author: somebyte
 */

#ifndef EVBTTY_EVBTTY_H_
#define EVBTTY_EVBTTY_H_

#include "../tty/uart.h"

void evbttyinit (uint32_t uartcfg);
void evbttymain ();

#define EVBTTY(UARTCFG) {\
      evbttyinit (UARTCFG);\
      while(1)\
        {\
          evbttymain ();\
        }\
}
#endif

