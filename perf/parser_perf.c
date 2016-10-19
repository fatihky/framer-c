#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>

#include <parser.h>

long long ustime(void) {
  struct timeval tv;
  long long ust;
  
  gettimeofday(&tv, NULL);
  ust = ((long long)tv.tv_sec)*1000000;
  ust += tv.tv_usec;
  return ust;
}

#define mstime() (ustime() /1000)

int main(int argc, char *argv[]) {
  // buffer setup
  char *buf = malloc(9 * 1e7);
  char *ptr = buf;
  int len = 5;
  for (int i = 0; i < 1e7; i++) {
    memcpy(ptr, &len, 4);
    memcpy(ptr + 4, "fatih", 5);
    ptr += 9;
  }
  // parser setup
  struct frm_parser parser;
  struct frm_cbuf cbuf;
  cbuf.buf = buf;
  int rc = frm_parser_init (&parser, 0);
  assert (rc == 0);
  int bufsz = 9 * 1e7;
  long long start = mstime();
  rc = frm_parser_parse (&parser, &cbuf, &bufsz);
  long long endms = mstime();
  assert (rc == 1e7);
  printf("parsing %d frames took %lldms\n", (int)1e7, endms - start);
  return 0;
}
