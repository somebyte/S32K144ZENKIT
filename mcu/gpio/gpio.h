/*
 * gpio.h
 *
 *  Created on: Aug 5, 2020
 *      Author: somebyte
 */

#ifndef GPIO_GPIO_H_
#define GPIO_GPIO_H_

#include <S32K144.h>

typedef enum
{
  nPTZ  = -1,
  nPTA  = 0,
  nPTB  = 1,
  nPTC  = 2,
  nPTD  = 3,
  nPTE  = 4,
  nSIZE = 5
} portnum_t;

PORT_Type* port_type_of      (portnum_t port);
GPIO_Type* gpio_type_of      (portnum_t port);
uint32_t   pcc_port_index_of (portnum_t port);

#define  ENABLE_CLOCK_FOR_PORT(PORTNAME) PCC->PCCn[PCC_PORT##PORTNAME##_INDEX]  =  PCC_PCCn_CGC_MASK;
#define DISABLE_CLOCK_FOR_PORT(PORTNAME) PCC->PCCn[PCC_PORT##PORTNAME##_INDEX] &= ~PCC_PCCn_CGC_MASK;

#define DEFINE_INPUT_PIN(PORTNAME,PINNUM) {\
  PT##PORTNAME->PDDR &= ~(uint32_t)(1<<PINNUM);\
  PORT##PORTNAME->PCR[PINNUM] = PORT_PCR_MUX(1);\
}

#define DEFINE_OUTPUT_PIN(PORTNAME,PINNUM,DATA) {\
  PT##PORTNAME->PDDR |= (uint32_t)(1<<PINNUM);\
  PORT##PORTNAME->PCR[PINNUM] = PORT_PCR_MUX(1);\
  if (DATA)\
    PT##PORTNAME->PSOR |= (uint32_t)(1<<PINNUM);\
  else\
    PT##PORTNAME->PCOR |= (uint32_t)(1<<PINNUM);\
}

#endif /* GPIO_GPIO_H_ */

