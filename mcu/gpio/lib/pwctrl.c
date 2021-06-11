/*
 * pwctrl.c
 *
 *  Created on: June 9, 2021
 *      Author: somebyte
 */

#include <S32K144.h>
#include <stdint.h>
#include <stddef.h>
#include "../pwctrl.h"
#include "../gpio.h"

int fstop  = 0;
int fhello = 0;

uint32_t nGPIOs[nSIZE] = {0,0,0,0,0};

PORT_Type* port_type_of(portnum_t);
GPIO_Type* gpio_type_of(portnum_t);

IO_t power_pins[LINES_NUMBER] = {
  { .port = nPTZ, .pin = -1, .off = -1 },
  { .port = nPTZ, .pin = -1, .off = -1 },
  { .port = nPTZ, .pin = -1, .off = -1 },
  { .port = nPTZ, .pin = -1, .off = -1 },
  { .port = nPTZ, .pin = -1, .off = -1 },
  { .port = nPTZ, .pin = -1, .off = -1 },
  { .port = nPTZ, .pin = -1, .off = -1 },
  { .port = nPTZ, .pin = -1, .off = -1 }
};

PORT_Type* IGNITION_KEY_PORT = NULL;
GPIO_Type* IGNITION_KEY_GPIO = NULL;
int        IGNITION_KEY_PIN  = -1;

static const    uint32_t n_key_max = 10;
static volatile uint32_t n_key_on  = 0;
static volatile uint32_t n_key_off = 0;

static int  __key_on  ();
static int  __key_off ();
static void __sleep   (void);

void
init_power_pins (void)
{
  for (size_t i = 1; i < LINES_NUMBER; ++i)
    {
      if (power_pins[i].port == nPTZ ||
	  power_pins[i].pin  <  0    ||
	  power_pins[i].pin  >= 32)
	continue;

      switch(power_pins[i].port)
	{
	case nPTA: DEFINE_OUTPUT_PIN(A, power_pins[i].pin, ((power_pins[i].off>0)?1:0))
        break;
	case nPTB: DEFINE_OUTPUT_PIN(B, power_pins[i].pin, ((power_pins[i].off>0)?1:0))
        break;
	case nPTC: DEFINE_OUTPUT_PIN(C, power_pins[i].pin, ((power_pins[i].off>0)?1:0))
        break;
	case nPTD: DEFINE_OUTPUT_PIN(D, power_pins[i].pin, ((power_pins[i].off>0)?1:0))
        break;
	case nPTE: DEFINE_OUTPUT_PIN(E, power_pins[i].pin, ((power_pins[i].off>0)?1:0))
        break;
	default: break;
	}
    }
  IGNITION_KEY_PORT = port_type_of(power_pins[IGNITION_KEY_LINE].port);
  IGNITION_KEY_GPIO = gpio_type_of(power_pins[IGNITION_KEY_LINE].port);
  IGNITION_KEY_PIN  = power_pins[IGNITION_KEY_LINE].pin;
  IGNITION_KEY_GPIO->PDDR &= ~(1<<IGNITION_KEY_PIN);
  IGNITION_KEY_PORT->PCR[IGNITION_KEY_PIN] |= PORT_PCR_MUX(1)|PORT_PCR_IRQC(0xC);
  IGNITION_KEY_PORT->DFCR  = PORT_DFCR_CS(1);
  IGNITION_KEY_PORT->DFWR  = PORT_DFWR_FILT(0x1F);
  IGNITION_KEY_PORT->DFER |= (uint32_t)(1<<IGNITION_KEY_PIN);
}

void
ignition_key_handle (ignkeyevent_t event)
{
  for (size_t i = 0; i < LINES_NUMBER; ++i)
    {
      switch(i)
        {
        case IGNITION_KEY_LINE:
        continue;
        default:
        break;
        }

      if ((event == IGNKEY_OFF) && (i == MPU_POWER_LINE)) // MPU not can be OFF
        continue;

      if (power_pins[i].port == nPTZ ||
      	  power_pins[i].pin  <  0    ||
      	  power_pins[i].pin  >= 32)
      	continue;

      if (event == IGNKEY_OFF)
        fhello = 0;

      GPIO_Type* ptr = gpio_type_of(power_pins[i].port);
      if (ptr)
        {
          int pcor = 0;
          switch (event)
            {
            case IGNKEY_ON:
              {
	            if (power_pins[i].off)
                  pcor = 1;
              }
            break;
            case IGNKEY_OFF:
              {
	            if (!power_pins[i].off)
                  pcor = 1;
              }
            break;
            default: break;
            }

          if (pcor)
            ptr->PCOR |= (uint32_t)(1<<power_pins[i].pin);
          else
	        ptr->PSOR |= (uint32_t)(1<<power_pins[i].pin);
        }
    }
}

void
IGNITION_KEY_PORT_IRQHandler (void)
{
  if (IGNITION_KEY_PIN < 0)
    return;

  if ((IGNITION_KEY_PORT->PCR[IGNITION_KEY_PIN] & PORT_PCR_ISF_MASK) >> PORT_PCR_ISF_SHIFT)
    {
      uint32_t key_on = !(IGNITION_KEY_GPIO->PDIR & (uint32_t)(1<<IGNITION_KEY_PIN));
      if (key_on)
        {
          IGNITION_KEY_PORT->PCR[IGNITION_KEY_PIN] &= ~PORT_PCR_IRQC(0x8);     // disabled IRQ Logic 0
          if (!__key_on())
	        {
	          IGNITION_KEY_PORT->PCR[IGNITION_KEY_PIN] |= PORT_PCR_IRQC(0x8);  // enabled IRQ Logic 0
	        }
          else
	        {
              IGNITION_KEY_PORT->PCR[IGNITION_KEY_PIN] |= PORT_PCR_IRQC(0xC);  // enabled IRQ Logic 1
            }
	      return;
        }
      else
      if (!key_on)
        {
	      IGNITION_KEY_PORT->PCR[IGNITION_KEY_PIN] &= ~PORT_PCR_IRQC(0xC);     // disabled IRQ Logic 1
	      if (!__key_off())
	        {
	          IGNITION_KEY_PORT->PCR[IGNITION_KEY_PIN] |= PORT_PCR_IRQC(0xC);  // enabled IRQ Logic 1
	        }
          else
            {
              IGNITION_KEY_PORT->PCR[IGNITION_KEY_PIN] |= PORT_PCR_IRQC(0x8);  // enabled IRQ Logic 0
            }
          return;
        }
    }
}

int
enter_STOP (void)
{
  if (fstop && fhello)
    {
      ignition_key_handle (IGNKEY_OFF);
      SMC->PMCTRL &= ~SMC_PMCTRL_STOPM_MASK;
      SMC->PMCTRL |=  SMC_PMCTRL_STOPM(1);
      (void)SMC->PMCTRL;
      __sleep ();
      return 1;
    }
  return 0;
}

int
__key_on()
{
  if (++n_key_on >= n_key_max)
    {
        n_key_on = 0;
        fstop  = 0;
        ignition_key_handle (IGNKEY_ON);
        return 1;
    }
    return 0;
}

int
__key_off()
{
  if (++n_key_off >= n_key_max)
    {
      n_key_off = 0;
      fstop = 1;
      return 1;
    }
  return 0;
}

void
__sleep(void)
{
  S32_SCB->SCR |= S32_SCB_SCR_SLEEPDEEP_MASK;
  asm("WFI");
}

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

