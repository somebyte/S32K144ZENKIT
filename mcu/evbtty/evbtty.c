#include "S32K144.h"

#include <string.h>
#include <stdio.h>
#include "lights.h"
#include "evbtty.h"
#include "../sys/clocks.h"
#include "../tty/tty.h"
#include "../tty/rpc.h"
#include "../adc/adc.h"

int led (const void *ptr);
int adc (const void *ptr);
int _extra_proctree();
int _extra_help();

int operating_adc = -1;

void
evbttyinit (uint32_t uartcfg)
{
  extra_proctree = _extra_proctree;
  extra_help     = _extra_help;

  init_CLKs ();
  lights_init();
  uart_init (uartcfg);
  proctree_init();
}

void
evbttymain ()
{
  char buffer[MAX_CANON] = {0};

  while (1)
    {
      if (uart_gets (buffer, MAX_CANON))
        {
          callproc (buffer);
          continue;
        }
      uart_putc (EOT);
    }
}

int
_extra_proctree ()
{
  if (!proctree_ptr)
    return -1;
  insert_command (proctree_ptr, "led", led);
  insert_command (proctree_ptr, "adc", adc);
  return 0;
}

int
_extra_help ()
{
  uart_puts ("\tled <color> <on|off>\r\n",               MAX_CANON);
  uart_puts ("\t    <color> = red|blue|green|off  \r\n", MAX_CANON);
  uart_puts ("\tadc <n> <ch>\r\n", MAX_CANON);
  uart_puts ("\t    <n>  - ADC number  \r\n", MAX_CANON);
  uart_puts ("\t    <ch> - ADC channel \r\n", MAX_CANON);
  uart_puts ("\r\n",                          MAX_CANON);
  return 0;
}

int
led (const void* ptr)
{
  if(!ptr)
    return -1;

  char color[7]= {0};
  char state[4]= {0};

  sscanf((const char*)ptr, "%6s %3s", color, state);

  int n = 0;
  lights_t light = LIGHTS_OFF;

  if(!strncmp(color, "red", 3))
    {
      n = 4;
      light = RED_LIGHT;
    }
  else
  if(!strncmp(color, "green", 5))
    {
      n = 6;
      light = GREEN_LIGHT;
    }
  else
  if(!strncmp(color, "blue", 4))
    {
      n = 5;
      light = BLUE_LIGHT;
    }
  if(!strncmp(color, "off", 3))
    {
      lights_off ();
      return 0;
    }

  if (!n)
    return -1;

  int i = 0;

  if(!strncmp(state, "on", 3))
    i = 1;

  lights_set (light, i);

  return 0;
}

int adc  (const void* ptr)
{
  if(!ptr)
    return -1;

  int adcn = 0;
  int chn  = 0;

  sscanf((const char*)ptr, "%6d %3d", &adcn, &chn);

  if (operating_adc != adcn)
    {
      adc_disable();
      switch (adcn)
        {
        case 0: adc_init(ADC0); break;
        case 1: adc_init(ADC1); break;
        }
      operating_adc = adcn;
    }

  char number[11] = {0};
  snprintf (number, 10, "%lu mV", adc_wt_conversion_chx(chn));

  uart_puts (number, MAX_CANON);
  return 0;
}

/*__INTERRUPT_SVC void SVC_Handler() {
    accumulator += counter;
}*/
