#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include "ttyspeed.h"
const char *usage = "\n\tuploader <tty port device> <baudrate> <srec>\n";

#define MAX_SREC 81
#define ERR_OK  0x41
#define ERR_CRC 0x45

#define MAX_STR 255

enum
{
    ARG_PROG  = 0,
    ARG_TTY   = 1,
    ARG_SPEED = 2,
    ARG_SREC  = 3
};

FILE*          ttyfp  = NULL;
FILE*          srecfp = NULL;
struct termios ts;
int            fd = -1;
char           ch = 0;

speed_t brate = B0;

int main (int argc, char **argv)
{
  if (argc < 4)
    {
      fprintf (stderr, "%s\n", usage);
      exit (1);
    }

  ttyfp = fopen (argv[ARG_TTY], "r+");
    
  if (ttyfp == NULL)
    {
      perror ("open tty");
      exit (2);
    }

  if (tcgetattr (fileno (ttyfp),  &ts) < 0)
    {
      perror ("get tty attr");
      exit (3);
    }

  brate = baudrate (argv[ARG_SPEED]);

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
  ts.c_cc[VTIME] = 3;
  ts.c_cc[VMIN]  = 0;

  if ((fd = fileno (ttyfp)) < 0)
    {
      perror ("get fileno");
      exit (5); 
    } 

  if (tcsetattr (fileno (ttyfp), TCSAFLUSH, &ts) < 0)
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

  srecfp = fopen (argv[ARG_SREC], "r");

  if (srecfp == NULL)
    {
      perror(argv[ARG_SREC]);
      exit(8);
    }

  fflush (ttyfp);
  if (fputs ("upload\n", ttyfp) == EOF)
    {
      perror("fputs");
      exit(9);
    }
 
  do
    {
      ch = fgetc (ttyfp);
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
      if (ch == (char) 0x41)
        {
          printf("OK\n");
          continue;
        }
      if (ch == (char) 0x45)
        {
          printf("CRC ERROR\n");
          break;
        }
      if (ch == (char) 0x04)
        {
          printf("^D CONTINUE\n");
        }
      else
        {
          char buf[MAX_STR];
          ungetc (ch, ttyfp);
          if (fgets(buf, MAX_STR, ttyfp) != NULL)
            fputs (buf, stdout);
          continue;
        }
      clearerr(ttyfp);
      fflush (ttyfp);
      char srecbuf[MAX_SREC] = {0};
      fgets (srecbuf, MAX_SREC, srecfp);
      printf ("%s", srecbuf);
      if (strlen(srecbuf) > 0)
        fputs (srecbuf, ttyfp);
      srecbuf[0] = 0;
    }
  while (feof(srecfp) == 0);

  fclose (ttyfp);
  fclose (srecfp);
  return 0;
}
