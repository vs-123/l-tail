#include <stdio.h>

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

int
main (void)
{
   printf ("Hello, World!\n");
   return 0;
}
