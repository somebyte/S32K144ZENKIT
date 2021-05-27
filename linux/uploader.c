#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include "ttyconfig.h"
const char *usage = "\n\tuploader <tty port device> <baudrate> <srec>\n";

#define MAX_SREC 81
#define ERR_OK  0x41
#define ERR_CRC 0x45

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

  brate = baudrate (argv[ARG_SPEED]);

  ttyfp = open_tty (argv[ARG_TTY], brate);
  if (ttyfp == NULL)
    exit(1);

  setbuf (ttyfp, NULL);

  srecfp = fopen (argv[ARG_SREC], "r");
  if (srecfp == NULL)
    {
      perror(argv[ARG_SREC]);
      exit(2);
    }

  fflush (ttyfp);
  if (fputs ("upload\n", ttyfp) == EOF)
    {
      perror("fputs");
      exit(9);
    }

  int fstop = 0;
  while (feof(srecfp) == 0)
    {
      ch = fgetc (ttyfp);
      switch (ch)
        {
        case EOF:
          printf("EOF\n");
          fstop = 1;
          break;
        case NUL:
          printf("NULL\n");
          continue;
        case ACK:
          printf("ACK\n");
          break;
        case CAN: 
          printf("CANCEL\n");
          fstop = 1;
          break;
        case EOT:
          printf("EOT\n");
          fstop = 1;
          break;
        default:
          ungetc (ch, ttyfp);
          char buf[_POSIX_MAX_CANON];
          if (fgets(buf, _POSIX_MAX_CANON, ttyfp) != NULL)
            {
              fputs (buf, stdout);
              continue;
            }
          break;
        }

      if (fstop)
        {
          break;
        }

      clearerr(ttyfp);
      fflush  (ttyfp);

      char srecbuf[MAX_SREC] = {0};
      fgets (srecbuf, MAX_SREC, srecfp);
      if (strlen (srecbuf) > 0)
        {
          printf ("SREC: %s", srecbuf);
          fputs (srecbuf, ttyfp);
        }
    }

  fclose (ttyfp);
  fclose (srecfp);
  return 0;
}
