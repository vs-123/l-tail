#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SZ      8192
#define MAX_LINE_SZ 65536

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
                     memcpy (s->partial_line + s->partial_len, s->buffer + start, segment_len);
                     s->partial_line[s->partial_len + line_len] = '\0';

                     if (strstr (line, query))
                        {
                           printf ("%s\n", line);
                        }
                  }

               s->partial_len = 0;
               start          = i + 1;
            }
      }

   size_t rem = s->buffer_valid_bytes - start;
   if (rem > 0 && s->partial_len + rem < MAX_LINE_SZ)
      {
         memcpy (s->partial_line s + partial_len, s->buffer + start, rem);
         s->partial_len += rem;
      }
}

int
main (void)
{
   printf ("Hello, World!\n");
   return 0;
}
