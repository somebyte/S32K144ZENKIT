/*
 * rpc.c
 *
 *  Created on: Feb 20, 2021
 *      Author: somebyte
 */
#include <stdio.h>
#include <string.h>

#include "../rpc.h"
#include "../tty.h"
#include "../uart.h"

cmdtree_ptr_t proctree_ptr = NULL;

static char procname[MAX_CANON] = {0};
static char _buffer[MAX_CANON] = {0};
char *rpcbuffer = _buffer;

extra_ptr_t extra_proctree = NULL;
extra_ptr_t extra_help     = NULL;

int help         (const void* ptr);
int unknown_proc (const char* procname);

cmdtree_ptr_t
proctree_init ()
{
  if (proctree_ptr)
    return proctree_ptr;

  proctree_ptr = create_cmdtree();
  insert_command (proctree_ptr, "help", help);

  if (extra_proctree != NULL)
    extra_proctree();

  return proctree_ptr;
}

int
callproc (const char* instruction)
{
    int res = -1;

    if (!instruction)
      {
	return res;
      }

    procname[0] = 0;
    sscanf (instruction, "%s", procname);

    cmdfunc_ptr_t ptr = search_command (proctree_ptr, procname);

    if (!ptr)
        res = unknown_proc (procname);
    else
      {
	uint8_t l = strlen(procname);
        if (strlen(instruction) > l)
          res = (*ptr)(&instruction[l]);
        else
          res = (*ptr)(NULL);
      }

    uart_putc (EOT);
    return res;
}

int
help (const void* ptr)
{
  uart_puts ("\thelp - print this text\r\n",   MAX_CANON);

  if (extra_help != NULL)
    extra_help();

  return 0;
}

int
unknown_proc (const char* procname)
{
  snprintf  (rpcbuffer, MAX_CANON, "WARNING: command \"%s\" is unknown\r\n", procname);
  uart_puts (rpcbuffer, MAX_CANON);
  return -1;
}

