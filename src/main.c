#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SZ      8192
#define MAX_LINE_SZ 262144

typedef struct
{
   int fd;
   char buffer[BUF_SZ];
   size_t buffer_valid_bytes;
   size_t pos;

   char partial_line[MAX_LINE_SZ];
   size_t partial_len;
} scanner_t;

void
scanner_init (scanner_t *s, int fd)
{
   s->fd                 = fd;
   s->buffer_valid_bytes = 0;
   s->partial_len        = 0;
}

void
process_buffer (scanner_t *s, const char *query)
{
   size_t start = 0;

   for (size_t i = 0; i < s->buffer_valid_bytes; i++)
      {
         if (s->buffer[i] == '\n')
            {
               if (s->partial_len >= MAX_LINE_SZ)
                  {
                     s->partial_len = 0;
                     start          = i + 1;
                     continue;
                  }

               size_t segment_len = i - start;

               if (s->partial_len < MAX_LINE_SZ
                   && (s->partial_len + segment_len < MAX_LINE_SZ))
                  {
                     memcpy (s->partial_line + s->partial_len,
                             s->buffer + start, segment_len);
                     s->partial_line[s->partial_len + segment_len] = '\0';

                     if (strstr (s->partial_line, query))
                        {
                           printf ("%s\n", s->partial_line);
                        }
                  }

               s->partial_len = 0;
               start          = i + 1;
            }
      }

   if (s->partial_len >= MAX_LINE_SZ)
      {
         return;
      }

   size_t rem = s->buffer_valid_bytes - start;
   if (rem > 0)
      {
         if (s->partial_len + rem < MAX_LINE_SZ)
            {
               memcpy (s->partial_line + s->partial_len, s->buffer + start,
                       rem);
               s->partial_len += rem;
            }
         else
            {
               fprintf (stderr,
                        "[L-TAIL WARNING] LINE TRUNCATED AS IT EXCEEDS %d BYTES.\n",
                        MAX_LINE_SZ);
               s->partial_len = MAX_LINE_SZ;
            }
      }
}

volatile sig_atomic_t should_poll = 1;

void
sigint_handler (int signal)
{
   (void)signal;
   should_poll = 0;
   fprintf (stderr, "[L-TAIL ERROR] CAUGHT SIGINT\n");
}

int
strncmpci (char const *str1, char const *str2, unsigned int n)
{
   for (unsigned int i = 0; i < n; i++)
      {
         int cmp = tolower (*str1) - tolower (*str2);
         if (!*str1 || !*str2 || cmp != 0)
            {
               return cmp;
            }
         str1++, str2++;
      }

   return 0;
}

void
print_usage (const char *program_name)
{
#define popt(opt, desc) printf ("   %-45s %s\n", opt, desc)
   printf ("[USAGE] %s [FLAGS] [<FILE_PATH> <QUERY_STRING>]\n", program_name);
   printf ("[FLAGS]\n");
   popt ("--HELP, -H, -?", "PRINT THIS HELP MESSAGE AND EXIT");
   popt ("--INFO, -I", "PRINT INFORMATION ABOUT THIS PROGRAM AND EXIT");
   popt ("--ONCE, -O <FILE_PATH> <QUERY_STRING>",
         "DISABLE POLLING, TERMINATE UPON REACHING EOF");
   printf ("[NOTE] FLAGS ARE NOT CASE-SENSITIVE\n");
#undef popt
}

void
print_info (void)
{
#define pinfo(aspect, detail) printf ("   * %-17s %s\n", aspect, detail)
   printf ("[INFO]\n");
   printf ("   L-TAIL -- A DEAD-SIMPLE, MEMORY-EFFICIENT, POSIX-COMPLIANT, "
           "HIGH-PERFORMANCE, BUFFERED LOG-SCANNING UTILITY TOOL FOR "
           "REAL-TIME, NON-BLOCKING STREAM MONITORING.\n");
   printf ("\n");
   pinfo ("[AUTHOR]", "vs-123 @ https://github.com/vs-123");
   pinfo ("[REPOSITORY]", "https://github.com/vs-123/l-tail");
   pinfo ("[LICENSE]",
          "GNU AFFERO GENERAL PUBLIC LICENSE VERSION 3.0 OR LATER");
#undef pinfo
}

int
main (int argc, char **argv)
{
   const char *program_name = argv[0];
   char *filepath           = argv[1];
   char *query              = argv[2];
   int should_pass_once     = 0;

   if (argc < 2)
      {
         print_usage (program_name);
         return 1;
      }
   else if (strncmpci (argv[1], "-h", 2) == 0
            || strncmpci (argv[1], "-?", 2) == 0
            || strncmpci (argv[1], "--help", 6) == 0)
      {
         print_usage (program_name);
         return 0;
      }
   else if (strncmpci (argv[1], "-i", 2) == 0
            || strncmpci (argv[1], "--info", 6) == 0)
      {
         print_info ();
         return 0;
      }
   else if (argc < 4)
      {
         fprintf (stderr, "[L-TAIL ERROR] MALFORMED ARGUMENTS, USE --HELP\n");
         return 1;
      }
   else if (strncmpci (argv[1], "-o", 2) == 0
            || strncmpci (argv[1], "--once", 6) == 0)
      {

         filepath         = argv[2];
         query            = argv[3];
         should_pass_once = 1;
      }
   else if (argv[1][0] == '-')
      {
         fprintf (stderr, "[L-TAIL ERROR] INVALID FLAG, USE --HELP\n");
         return 1;
      }

   int fd = open (filepath, O_RDONLY);
   if (fd < 0)
      {
         fprintf (stderr, "[L-TAIL ERROR] COULD NOT OPEN\n");
         return 1;
      }

   scanner_t s;
   scanner_init (&s, fd);

   signal (SIGINT, sigint_handler);

   while (should_poll)
      {
         ssize_t bytes_read = read (fd, s.buffer, BUF_SZ);
         if (bytes_read > 0)
            {
               s.buffer_valid_bytes = (size_t)bytes_read;
               process_buffer (&s, query);
            }
         else if (bytes_read == 0)
            {
               /* reached EOF */
               if (should_pass_once)
                  {
                     break;
                  };
               usleep (100000);
            }
         else
            {
               fprintf (stderr, "[L-TAIL ERROR] COULD NOT READ\n");
               break;
            }
      }

   close (fd);
   printf ("[L-TAIL INFO] EXITING...\n");

   return 0;
}
