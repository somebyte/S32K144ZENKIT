#ifndef __TTYSPEED_H__
#define __TTYSPEED_H__

#include <termios.h>
#include <stdio.h>

speed_t baudrate(const char* from);

FILE* open_tty (const char* filepath, speed_t baudrate);

#endif

