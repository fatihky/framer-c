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

// static struct frm_frame fakeframe;
static struct frm_frame *frames = NULL;
static int framecursor = 0;

struct frm_frame *frm_cb (struct frm_parser *self) {
  (void)self;
  return &frames[framecursor++];
  // return &fakeframe;
}

int main(int argc, char *argv[]) {
  // buffer setup
  char *buf = malloc(9 * 2e7);
  char *ptr = buf;
  int len = 5;
  for (int i = 0; i < 2e7; i++) {
    memcpy(ptr, &len, 4);
    memcpy(ptr + 4, "fatih", 5);
    ptr += 9;
  }
  // parser setup
  struct frm_parser parser;
  struct frm_cbuf cbuf;
  frames = malloc(sizeof (struct frm_frame) * 2e7);
  cbuf.buf = buf;
  int rc = frm_parser_init (&parser, 1);
  assert (rc == 0);
  parser.frm_cb = frm_cb;
  int bufsz = 9 * 2e7;
  long long start = mstime();
  rc = frm_parser_parse (&parser, &cbuf, &bufsz);
  long long endms = mstime();
  assert (rc == 2e7);
  printf("parsing %d frames took %lldms\n", (int)2e7, endms - start);
  return 0;
}
