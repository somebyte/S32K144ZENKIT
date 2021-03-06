#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include "ttyconfig.h"

const char *usage = "\n\tttydebuger <tty port device> [baudrate]\n";

enum
{
    ARG_PROG  = 0,
    ARG_TTY   = 1,
    ARG_SPEED = 2
};

FILE* fp = NULL;
int   fd = -1;

char inputs[_POSIX_MAX_CANON];
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

  fp = open_tty (argv[ARG_TTY], brate);

  if (!fp)
    exit(1);
  
  fprintf (stdout, "\n\tType 'quit' to finish.");
  fprintf (stdout, "\n\tType 'help' for a list of commands.\n\n");

  while (1)
    {
      fflush (fp);
      fflush (stdin);
      inputs[0] = 0;

      fgets (inputs, _POSIX_MAX_CANON, stdin);
      if (strcmp(inputs, "quit\n") == 0)
        {
          break;
        }

      while (fputs (inputs, fp) == EOF)
        {
          perror("fputs");
          fclose(fp);
          fp = NULL;
          if (errno == ESPIPE) /* it is may be, for example press RESET or jump to address*/
              fp = open_tty (argv[ARG_TTY], brate);
          if (!fp)
            exit(2);
          continue;
        }

      int fend = 0;
      while (!fend)
        {
          ch = fgetc (fp);
          switch (ch)
            {
            case (char) EOF:
              printf ("EOF\n");
              fend = 1;
              continue;
            case NUL:
              printf ("NULL\n");
              fend = 1;
              continue;
            case ETB:
              printf("ETB\n");
              continue;
            case ACK:
              printf("ACK\n");
              break;
            case CAN: 
              printf("CANCEL\n");
              fend = 1;
              break;
            case EOT:
              printf ("EOT\n");
              fend = 1;
              continue;
            }
          ungetc (ch, fp);
          inputs[0] = 0;
          if (fgets (inputs, _POSIX_MAX_CANON, fp) == NULL)
            {
              break;
            }
          fputs (inputs, stderr);
        }

      clearerr(fp);
    }

  fclose (fp);
  return 0;
}


