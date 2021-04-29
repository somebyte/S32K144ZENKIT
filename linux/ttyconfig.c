#include "ttyconfig.h"
#include <stdio.h>
#include <errno.h>

speed_t
baudrate (const char* from)
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

FILE*
open_tty (const char* filepath, speed_t brate)
{
  if (!filepath)
    return NULL;
 
  struct termios ts;
  int   fd = -1;
  FILE* fp = fopen (filepath, "r+");

  if (fp == NULL)
    {
      perror ("open tty");
      return NULL;
    }

  if (tcgetattr (fileno (fp),  &ts) < 0)
    {
      perror ("get tty attr");
      return NULL;
    }

  if (cfsetspeed (&ts, brate))
    {
      perror ("set tty speed");
      return NULL;
    }

  ts.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG);
  ts.c_iflag &= ~(BRKINT|ICRNL|INPCK|ISTRIP|IXON);
  ts.c_cflag &=
  ts.c_cflag |= CS8;
  ts.c_cflag &= ~(PARENB|CSIZE|CSTOPB);
  ts.c_oflag &= ~(OPOST);
  ts.c_cc[VTIME] = 1;
  ts.c_cc[VMIN]  = 0;

  if ((fd = fileno (fp)) < 0)
    {
      perror ("get fileno");
      fclose (fp);
      return NULL;
    } 

  if (tcsetattr (fileno (fp), TCSAFLUSH, &ts) < 0)
    {
      perror ("set tty attr");
      fclose (fp);
      return NULL;
    }

  if ((ts.c_lflag & (ECHO|ICANON|IEXTEN|ISIG))        &&
      (ts.c_iflag & (BRKINT|ICRNL|INPCK|ISTRIP|IXON)) &&
     ((ts.c_cflag & (CS8|PARENB|CSIZE|CSTOPB)) !=CS8) &&
      (ts.c_oflag & OPOST)
     )
    {
      errno = EINVAL;
      perror ("check set tty attr");
      fclose (fp);
      return NULL;
    }

  return fp;
}
