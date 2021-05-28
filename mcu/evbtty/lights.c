/*
 * lights.c
 *
 *  Created on: May 12, 2021
 *      Author: somebyte
 */

#include "S32K144.h"
#include "lights.h"

#define RED   15
#define GREEN 16
#define BLUE  0

void
lights_init()
{
  PCC-> PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORT D */
  PTD->PDDR |= 1<<RED;
  PTD->PDDR |= 1<<GREEN;
  PTD->PDDR |= 1<<BLUE;
  PORTD->PCR[RED]   |=  PORT_PCR_MUX(1);
  PORTD->PCR[GREEN] |=  PORT_PCR_MUX(1);
  PORTD->PCR[BLUE]  |=  PORT_PCR_MUX(1);
  lights_off();
}

void
lights_off ()
{
  PTD->PSOR |= 1<<RED | 1<<GREEN | 1<<BLUE; /* turn off red, green LEDs */
}

void
lights_set (lights_t light, int state )
{
  if (state)
    {
      switch (light)
        {
        case RED_LIGHT:   PTD->PCOR |= 1<<RED;   break;
        case GREEN_LIGHT: PTD->PCOR |= 1<<GREEN; break;
        case BLUE_LIGHT:  PTD->PCOR |= 1<<BLUE;  break;
        default: break;
        }
    }
  else
    {
      switch (light)
        {
        case RED_LIGHT:   PTD->PSOR |= 1<<RED;   break;
        case GREEN_LIGHT: PTD->PSOR |= 1<<GREEN; break;
        case BLUE_LIGHT:  PTD->PSOR |= 1<<BLUE;  break;
        default: break;
        }
    }
}
