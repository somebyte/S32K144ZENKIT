#include "ttyspeed.h"
#include <stdio.h>

speed_t baudrate(const char* from)
{
  speed_t _baudrate = B0;
  sscanf (from, "%u", &_baudrate);
  switch (_baudrate)
  {
  case   9600: _baudrate = B9600;   break;
  case  19200: _baudrate = B19200;  break;
  case  38400: _baudrate = B38400;  break;
  case  57600: _baudrate = B57600;  break;
  case 115200: _baudrate = B115200; break;
  case 230400: _baudrate = B230400; break;
  case 460800: _baudrate = B460800; break;
  case 921600: _baudrate = B921600; break;
  }
  return _baudrate;
}


