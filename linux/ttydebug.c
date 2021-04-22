#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include "ttyspeed.h"

const char *usage = "\n\tttydebuger <tty port device> [baudrate]\n";

enum
{
    ARG_PROG  = 0,
    ARG_TTY   = 1,
    ARG_SPEED = 2
};

FILE*          fp = NULL;
struct termios ts;
int            fd = -1;

#define MAX_INPUT_STR 1024
char inputs[MAX_INPUT_STR];
char ch = 0;

speed_t brate = B0;

int main (int argc, char **argv)
{
  if (argc < 2)
    {
      fprintf (stderr, "%s\n", usage);
      exit (1);
    }

  if (argc > 2)
    {
      brate = baudrate(argv[ARG_SPEED]);
    }  

  fp = fopen (argv[ARG_TTY], "r+");
    
  if (fp == NULL)
    {
      perror ("open tty");
      exit (2);
    }

  if (tcgetattr (fileno (fp),  &ts) < 0)
    {
      perror ("get tty attr");
      exit (3);
    }

  if (cfsetspeed (&ts, brate))
    {
      perror ("set tty speed");
      exit (4);
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
      exit (5); 
    } 

  if (tcsetattr (fileno (fp), TCSAFLUSH, &ts) < 0)
    {
      perror ("set tty attr");
      exit (6);
    }

  if ((ts.c_lflag & (ECHO|ICANON|IEXTEN|ISIG))        &&
      (ts.c_iflag & (BRKINT|ICRNL|INPCK|ISTRIP|IXON)) &&
     ((ts.c_cflag & (CS8|PARENB|CSIZE|CSTOPB)) !=CS8) &&
      (ts.c_oflag & OPOST)
     )
    {
      errno = EINVAL;
      perror ("check set tty attr");
      exit (7);
    }

  fprintf (stdout, "\n\tType 'quit' to finish.");
  fprintf (stdout, "\n\tType 'help' for a list of commands.\n\n");

  while (1)
    {
      fflush (fp);
      fflush (stdin);
      inputs[0] = 0;

      fgets (inputs, MAX_INPUT_STR, stdin);
      if (strcmp(inputs, "quit\n") == 0)
        {
          break;
        }

      if (fputs (inputs, fp) == EOF)
        {
          perror("fputs");
        }

      while(1)
        {
          ch = fgetc (fp);
          if (ch == (char) EOF)
            {
              printf("EOF\n");
              break;
            }
          if (ch == (char) 0)
            {
              printf("EMPTY\n");
              break;
            }
          if (ch == (char) 0x04)
            {
              printf("^D\n");
              break;
            }
          ungetc (ch, fp);
          inputs[0] = 0;
          if (fgets (inputs, MAX_INPUT_STR, fp) == NULL)
            break;
          fputs (inputs, stderr);
        }

      clearerr(fp);
    }

  fclose (fp);
  return 0;
}


