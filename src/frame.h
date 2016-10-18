#include <stddef.h>
#include "cbuf.h"

#ifndef FRM_FRAME_INCLUDED
#define FRM_FRAME_INCLUDED

enum frm_frame_type {
  FRM_FRAME_EMPTY,
  FRM_FRAME_ALLOCATED, // free frame's `buf` at termination
  FRM_FRAME_EMBEDDED  // frame's buffer is embedded so don't touch its `buf`
                      // call `frm_cbuf_unref`
};

struct frm_frame {
  // ro
  int size; // it can be also used as `capacity` if needed. i.e: for reads
  char *buf;
  // private
  enum frm_frame_type type;
  struct frm_cbuf *cbuf;
  int ref;
  // rw
  void *data;
  void (*free_cb) (struct frm_frame *self);
};

void frm_frame_init(struct frm_frame *self);
void frm_frame_term(struct frm_frame *self);
int frm_frame_totlen(struct frm_frame *self);

/**
 * This function calls `_term` method if reference count is equal
 * to zero. This function DOES NOT free the frame object.
 * If you want to do, set `free_cb`. You can set it to `free`.
 */
void frm_frame_unref(struct frm_frame *self);

#endif

/*

- out list
- recv list
- in list

okumalar soyle olmali:
char *buf = malloc(1000);
ssize_t nread = read(fd, buf, 1000);
parse(parser, buf, nread);

parcalayici frameleri bizden almali. biz saglamiyorsak mallocla olusturmali.
peki liste nasil olacak?
yine liste olsun. liste saglama fonksiyonu da olsun.



okumalar direkt bos listeye yapilsin.
zaten listede cursor olayi var.
kullanici listeden verileri okumaya calisinca cursor degeri -1 olanlari atlansin
ama, ne bu atlama ki? ne zaman atlanir? mesela list_pop cagrildiginda cursor
degeri -1 yapilir bu sayede kapali oldugu anlasilir. eger elemanin cursor degeri
boyut degeri ile uyusmuyorsa daha tamami okunmamis demektir, bu durumda atlanir.



*/
