/*
 * ftm.h
 *
 *  Created on: Jun 22, 2021
 *      Author: somebyte
 */

#ifndef FTM_FTM_H_
#define FTM_FTM_H_

#include <stdint.h>
#include "../gpio/gpio.h"

typedef enum
{
  FTM_N0 = 0,
  FTM_N1 = 1,
  FTM_N2 = 2,
  FTM_N3 = 3,
  FTM_NS = 4
} ftmnum_t;

int      ftm_init  (ftmnum_t  FTMn,
	            uint32_t  CHx,
	            portnum_t PORTn,
	            uint32_t  GPIOn,
	            uint32_t  ALT);
void     ftm_start (ftmnum_t  FTMn);
uint16_t ftm_ticks (ftmnum_t  FTMn);

#endif /* FTM_FTM_H_ */
