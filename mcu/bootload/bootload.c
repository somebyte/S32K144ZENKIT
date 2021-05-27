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
#include "../tty/tty.h"
#include "../tty/rpc.h"
#include "../fw/fw.h"

bootload_t _btlstate = BOOTL_RESETED;

int _extra_proctree();
int _extra_help();
int _upload (const void*);
int _jump   (const void*);

bootload_t bootloadinit (uint32_t uartcfg)
{
  if ((_btlstate != BOOTL_JUMP) &&
      (_btlstate != BOOTL_RESETED))
    return _btlstate;

  extra_proctree = _extra_proctree;
  extra_help     = _extra_help;

  init_CLKs ();
  uart_init (uartcfg);
  proctree_init();

  _btlstate = BOOTL_INITED;
  return _btlstate;
}

bootload_t bootloadreset()
{
  extra_proctree = NULL;
  extra_help     = NULL;

  proctree_reset();
  uart_reset   ();
  disable_CLKs();

  _btlstate = BOOTL_RESETED;
  return _btlstate;
}

bootload_t
bootloadmain ()
{
  char buffer[MAX_CANON] = {0};

  while ((_btlstate != BOOTL_JUMP) &&
         (_btlstate != BOOTL_RESETED))
    {
      if (uart_gets (buffer, MAX_CANON))
        {
          callproc (buffer);
          continue;
        }

      uart_putc (EOT);
    }

  return _btlstate;
}

int
_extra_proctree ()
{
  if (!proctree_ptr)
    return -1;
  insert_command (proctree_ptr, "upload", _upload);
  insert_command (proctree_ptr, "jump",   _jump);
  return 0;
}

int
_extra_help ()
{
  uart_puts ("\tupload\r\n",                  MAX_CANON);
  uart_puts ("\t  /* transmit fw to mcu  */\r\n",  MAX_CANON);
  uart_puts ("\tjump APP_BEGIN_ADDRESS\r\n",  MAX_CANON);
  uart_puts ("\t  /* jump to application */\r\n", MAX_CANON);
  uart_puts ("\r\n",                          MAX_CANON);
  return 0;
}

int
_upload (const void* ptr)
{
  //uart_puts   ("READY", MAX_CANON);
  uart_putc   (ACK);
  download_fw ();
  return 0;
}

int
_jump (const void* ptr)
{
  if (ptr)
      sscanf( (const char*)ptr, "%lX", &APP_BEGIN_ADDRESS);
  uart_putc(EOT);
  _btlstate = BOOTL_JUMP;
  return 0;
}


