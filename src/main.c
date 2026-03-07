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

void process_buffer(scanner_t *s, const char *query);

int
main (void)
{
   printf ("Hello, World!\n");
   return 0;
}
