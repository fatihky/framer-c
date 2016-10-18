#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "util.h"

static inline struct frm_frame *_create_frame_copy(struct frm_frame *src)
                                        __attribute__((always_inline));
static inline int _push_frame_if_complete(struct frm_parser *self, int *frm_cursor,
                  struct frm_frame *src, int *bufsz, int remaining) __attribute__((always_inline));

int frm_parser_init (struct frm_parser *self, int embed_allow) {
  int rc;
  self->curr_cursor = 0;
  self->embed_allow = embed_allow;
  frm_frame_init(&self->curr_frame);
  rc = frm_fl_init (&self->in_frames, 100);
  return rc;
}

static inline struct frm_frame *_create_frame_copy(struct frm_frame *src) {
  struct frm_frame *copy = malloc(sizeof (struct frm_frame));
  if (copy == NULL)
    return NULL;

  memcpy(copy, src, sizeof (struct frm_frame));

  return copy;
}

static inline int _push_frame_if_complete(struct frm_parser *self,
    int *frm_cursor, struct frm_frame *fr, int *bufsz, int remaining) {

  struct frm_frame *copy;

  if ((*frm_cursor - 4) == fr->size) {
    // allocate new frame and copy current frame's contents to it
    copy = _create_frame_copy(fr);
    if (copy == NULL)
      goto fail1;

    bool res = frm_fl_add (&self->in_frames, copy);
    if (res == false)
      goto fail2;

    // reset the old frame
    frm_frame_init(fr);
    *frm_cursor = 0;
    return 1;
  }

  return 0;

fail2:
  free (copy);
fail1:
  *bufsz = remaining;
  errno = ENOMEM;
  return -1;
}

/**
 * WARNING: please don't give real buf size variable's pointer as `bufsz`
 * parser sets its value on error. so you can continue parsing after error.
 * just give copy's pointer.
 */

int frm_parser_parse (struct frm_parser *self, struct frm_cbuf *cbuf, int *bufsz) {
  int rc;
  int towrite;
  int needed;
  int remaining = *bufsz;
  int embed_allow = self->embed_allow;
  struct frm_frame *fr = &self->curr_frame;
  char *ptr = cbuf->buf;
  int *frm_cursor = &self->curr_cursor;
  int parsed_frames = 0;

  rc = _push_frame_if_complete(self, frm_cursor, fr, bufsz, remaining);
  if (rc == -1)
    return rc;

  for (; remaining > 0;) {
    if (*frm_cursor < 4) {
      needed = 4 - *frm_cursor;
      towrite = frm_min(remaining, needed);
      if (towrite == 4)
        fr->size = *(int *)ptr;
      else
        memcpy(((char *)&fr->size) + *frm_cursor, ptr, towrite);
      // update cursor
      *frm_cursor += towrite;
      remaining -= towrite;
      ptr += towrite;
    }

    if (remaining == 0 || *frm_cursor < 4) break;

    needed = fr->size - *frm_cursor + 4;
    towrite = frm_min(remaining, needed);

    if (fr->size == towrite && embed_allow) {
      frm_cbuf_ref(cbuf);
      fr->buf = ptr;
      fr->type = FRM_FRAME_EMBEDDED;
    }
    else {
      if (fr->buf == NULL) {
        fr->buf = malloc(fr->size);
        if (fr->buf == NULL) {
          *bufsz = remaining;
          return ENOMEM;
        }
        fr->type = FRM_FRAME_ALLOCATED;
      }
      memcpy(fr->buf + *frm_cursor - 4, ptr, towrite);
    }

    ptr += towrite;
    *frm_cursor += towrite;
    remaining -= towrite;
    rc = _push_frame_if_complete(self, frm_cursor, fr, bufsz, remaining);
    if (rc == 1)
      parsed_frames++;
    else if (rc != 0)
      return rc;
  }

  return parsed_frames;
}

bool frm_parser_has_frame (struct frm_parser *self) {
  return !frm_fl_is_empty (&self->in_frames);
}
