#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SZ 8192

typedef struct
{
   int fd;
   char buffer[BUF_SZ];
   size_t buffer_valid_bytes;
   size_t pos;

   char *partial_line;
   size_t partial_len;
   size_t partial_cap;
} scanner_t;

void
process_buffer (scanner_t *s, const char *query)
{
   size_t start = 0;

   for (size_t i = 0; i < s->buffer_valid_bytes; i++)
      {
         if (s->buffer[i] == '\n')
            {
               size_t segment_len = i - start;

               size_t total_len = s->partial_len + segment_len;
               char *line       = malloc (total_len + 1);

               memcpy (line, s->partial_line, s->partial_len);
               memcpy (line + s->partial_len, &s->buffer[start], segment_len);
               line[total_len] = '\0';

               if (strstr (line, query))
                  {
                     printf("%s\n", line);
                  }

               free (line);
               s->partial_len = 0;
               start = i + 1;
            }
      }

   /**
      [TODO]
      - handle reamining bytes
      - move =s->buffer[start..valid_bytes]= to =s->partial_line=
   */
}

int
main (void)
{
   printf ("Hello, World!\n");
   return 0;
}
