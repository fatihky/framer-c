#include <stdlib.h>
#include <assert.h>
#include "frame.h"

void frm_frame_init (struct frm_frame *self) {
  self->size = 0;
  self->buf = NULL;
  self->cbuf = NULL;
  self->type = FRM_FRAME_EMPTY;
  self->ref = 0;
  self->data = NULL;
  self->free_cb = NULL;
}

void frm_frame_term(struct frm_frame *self) {
  assert (self->ref == 0);
  if (self->type == FRM_FRAME_EMBEDDED)
    frm_cbuf_unref(self->cbuf);
  else if (self->type == FRM_FRAME_ALLOCATED)
    free (self->buf);

  self->type = FRM_FRAME_EMPTY;
  // we don't need to set `buf` or `cbuf` to NULL since we just
  // need to check `type` to de-allocate resources
  self->size = 0;
  // we already checked the `ref == 0`
  // self->ref = 0;
}

int frm_frame_totlen(struct frm_frame *self) {
  return self->size + 4; // size + sizeof(int)
}

void frm_frame_ref(struct frm_frame *self) {
  assert (self->ref >= 0);
  self->ref++;
}

void frm_frame_unref(struct frm_frame *self) {
  assert (self->ref >= 0);
  self->ref--;
  if (self->ref > 0) return;
  if (self->free_cb)
    return self->free_cb(self);
}
