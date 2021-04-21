/*
 * nvic.c
 *
 *  Created on: Jul 31, 2020
 *      Author: somebyte
 */

#include <S32K144.h>
#include "../nvic.h"

void
NVIC_IRQn_init (uint8_t IRQN, uint8_t PR)
{
  uint8_t div = IRQN / 32;
  uint8_t mod = IRQN % 32;
  S32_NVIC->IP[ IRQN ] = PR;          /* IRQn: priority of 0-15 */
  S32_NVIC->ICPR[ div ] |= 1 << mod;  /* IRQn: clr any pending IRQ */
  S32_NVIC->ISER[ div ] |= 1 << mod;  /* IRQn: enable IRQ          */
}
