/*
 * gpio.c
 *
 *  Created on: Jun 22, 2021
 *      Author: somebyte
 */

#include <stddef.h>
#include "../gpio.h"

PORT_Type*
port_type_of (portnum_t port)
{
  switch(port)
    {
    case nPTA: return PORTA;
    case nPTB: return PORTB;
    case nPTC: return PORTC;
    case nPTD: return PORTD;
    case nPTE: return PORTE;
    default: return NULL;
    }
  return NULL;
}

GPIO_Type*
gpio_type_of (portnum_t port)
{
  switch(port)
    {
    case nPTA: return PTA;
    case nPTB: return PTB;
    case nPTC: return PTC;
    case nPTD: return PTD;
    case nPTE: return PTE;
    default: return NULL;
    }
  return NULL;
}

uint32_t
pcc_port_index_of (portnum_t port)
{
  switch(port)
    {
    case nPTA: return PCC_PORTA_INDEX;
    case nPTB: return PCC_PORTB_INDEX;
    case nPTC: return PCC_PORTC_INDEX;
    case nPTD: return PCC_PORTD_INDEX;
    case nPTE: return PCC_PORTE_INDEX;
    default: return NULL;
    }
  return NULL;
}


