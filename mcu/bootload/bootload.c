/*
 * bootload.c
 *
 *  Created on: Apr 21, 2021
 *      Author: somebyte
 */

#include <stdio.h>
#include <S32K144.h>
#include "bootload.h"
#include "../sys/clocks.h"
#include "../tty/uart.h"
#include "../tty/tty.h"
#include "../tty/rpc.h"
#include "../fw/fw.h"

#define UARTCFG UART_IFC2|UART_PIN_RX1|UART_PIN_TX1|UART_B921600

int need_jump_to_fw = 0;

int _extra_proctree();
int _extra_help();

int
bootloadmain ()
{
  extra_proctree = _extra_proctree;
  extra_help     = _extra_help;

  char buffer[MAX_CANON] = {0};

  init_CLKs ();
  uart_init (UARTCFG);
  proctree_init();

  while (1)
    {
      if (need_jump_to_fw)
      	{
      	  uart_reset   ();
      	  disable_CLKs ();
          jump_to_fw   ();
          init_CLKs ();
          uart_init (UARTCFG);
          need_jump_to_fw = 0;
      	}

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
  insert_command (proctree_ptr, "upload", upload);
  insert_command (proctree_ptr, "jump",   jump);
  return 0;
}

int
_extra_help ()
{
  uart_puts ("\tupload\r\n\t\t- transmit fw to mcu\r\n",  MAX_CANON);
  uart_puts ("\tjump APP_BEGIN_ADDRESS\r\n\t\t- jump to application\r\n", MAX_CANON);
  uart_puts ("\r\n", MAX_CANON);
  return 0;
}

int
upload (const void* ptr)
{
  uart_puts   ("READY", MAX_CANON);
  uart_putc   (EOT);
  download_fw ();
  return 0;
}

int
jump (const void* ptr)
{
  need_jump_to_fw = 1;
  if (ptr)
      sscanf( (const char*)ptr, "%X", &APP_BEGIN_ADDRESS);

  return 0;
}


