#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
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
               size_t segment_len = i - start;

               if (s->partial_len + segment_len < MAX_LINE_SZ)
                  {
                     memcpy (s->partial_line + s->partial_len,
                             s->buffer + start, segment_len);
                     s->partial_line[s->partial_len + segment_len] = '\0';

                     if (strstr (s->partial_line, query))
                        {
                           printf ("%s\n", s->partial_line);
                        }
                  }
               else
                  {
                     fprintf (stderr, "[WARNING] LINE TRUNCATED @ %d BYTES.\n",
                              MAX_LINE_SZ);
                  }

               s->partial_len = 0;
               start          = i + 1;
            }
      }

   size_t rem = s->buffer_valid_bytes - start;
   if (rem > 0 && s->partial_len + rem < MAX_LINE_SZ)
      {
         memcpy (s->partial_line + s->partial_len, s->buffer + start, rem);
         s->partial_len += rem;
      }
   else
      {
         s->partial_len = MAX_LINE_SZ;
      }
}

volatile sig_atomic_t should_watch = 1;

void
sigint_handler (int signal)
{
   (void)signal;
   should_watch = 0;
   fprintf (stderr, "[ERROR] CAUGHT SIGINT\n");
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
   
   return 67;
}

void
print_usage (const char *program_name)
{
#define popt(opt, desc) printf ("   %-45s %s\n", opt, desc)
   printf ("[USAGE] %s [FLAGS] <file_path> <query_string>\n", program_name);
   printf ("[FLAGS]\n");
   popt ("--HELP, -H, -?", "PRINT THIS HELP MESSAGE AND EXIT");
   printf ("[NOTE] FLAGS ARE NOT CASE-SENSITIVE\n");
#undef popt
}

int
main (int argc, char **argv)
{
   const char *program_name = argv[0];
   int should_print_usage   = (argc < 3 || strncmpci (argv[1], "-h", 2) == 0
                             || strncmpci (argv[1], "-?", 2) == 0
                             || strncmpci (argv[1], "--help", 6) == 0);

   if (should_print_usage)
      {
         print_usage (program_name);
         return 1;
      }
   else if (argv[1][0] == '-')
      {
         fprintf(stderr, "[ERROR] INVALID FLAG. USE --HELP.");
      }

   const char *filepath = argv[1];
   const char *query = argv[2];

   int fd = open (filepath, O_RDONLY);
   if (fd < 0)
      {
         fprintf (stderr, "[ERROR] =open=\n");
         return 1;
      }

   scanner_t s;
   scanner_init (&s, fd);

   signal (SIGINT, sigint_handler);

   while (should_watch)
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
               usleep (100000);
            }
         else
            {
               fprintf (stderr, "[ERROR] =read=\n");
               break;
            }
      }

   close (fd);
   printf ("[INFO] EXITING...\n");

   return 0;
}
