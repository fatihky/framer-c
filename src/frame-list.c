#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "frame-list.h"
#include "util.h"

static bool _frm_fl_add_bulk(struct frm_fl *self);
static inline void _frm_fl_bulk_remove_frame(struct frm_fl_bulk *self,
    int index) __attribute__((always_inline));

struct frm_fl_bulk *frm_fl_bulk_new (int capacity) {
  struct frm_fl_bulk *self = malloc(sizeof (struct frm_fl_bulk));
  if (self == NULL)
    goto fail1;

  struct frm_frame **frames = malloc(sizeof (void *) * capacity);
  if (frames == NULL)
    goto fail2;

  int *cursors = malloc(sizeof (int) * capacity);
  if (cursors == NULL)
    goto fail3;

  for (int i = 0; i < capacity; i++)
    cursors[i] = -1;

  self->frames = frames;
  self->cursors = cursors;
  self->lastidx = 0;
  self->size = 0;
  self->capacity = capacity;
  self->next = NULL;

  return self;

fail3:
  free(frames);
fail2:
  free(self);
fail1:
  errno = ENOMEM;
  return NULL;
}

void frm_fl_bulk_term (struct frm_fl_bulk *self) {
  free (self->frames);
  free (self->cursors);
  self->capacity = 0;
  self->size = 0;
}

void frm_fl_bulk_free (struct frm_fl_bulk *self) {
  frm_fl_bulk_term (self);
  free (self);
}

struct frm_fl *frm_fl_new (int capacity) {
  struct frm_fl *self;

  self = malloc(sizeof (struct frm_fl));
  if (self == NULL) {
    errno = ENOMEM;
    return NULL;
  }

  int rc = frm_fl_init(self, capacity);
  if (rc != 0) {
    errno = rc;
    free(self);
    return NULL;
  }

  return self;
}

int frm_fl_init (struct frm_fl *self, int capacity) {
  if (capacity <= 0) {
    return EINVAL;
  }

  self->head = NULL;
  self->tail = NULL;
  self->capacity = capacity;
  self->data = NULL;
  self->free_cb = NULL;

  return 0;
}

bool frm_fl_bulk_add (struct frm_fl_bulk *self, struct frm_frame *fr) {
  if (self->size == self->capacity)
    return false; // full

  frm_frame_ref(fr);
  self->frames[self->lastidx] = fr;
  self->cursors[self->lastidx] = 0;
  self->size++;
  self->lastidx++;

  return true;
}

static bool _frm_fl_add_bulk(struct frm_fl *self) {
  struct frm_fl_bulk *bulk = frm_fl_bulk_new(self->capacity);
  if (bulk == NULL)
    return false;
  if (self->head == NULL) {
    self->head = bulk;
  }
  else {
    if (self->tail == NULL)
      self->head->next = bulk;
    else
      self->tail->next = bulk;
    self->tail = bulk;
  }
  return true;
}

bool frm_fl_add (struct frm_fl *self, struct frm_frame *fr) {
  struct frm_fl_bulk *bulk;
  bool res;
  if (self->tail)
    bulk = self->tail;
  else if (self->head)
    bulk = self->head;
  else {
    res = _frm_fl_add_bulk(self);
    if (res == false)
      return false;
    return frm_fl_add(self, fr);
  }
  res = frm_fl_bulk_add(bulk, fr);
  if (res == false) {
    // create new bulk and try again
    res = _frm_fl_add_bulk(self);
    if (res == false)
      return false;
    return frm_fl_add(self, fr);
  }
  return true;
}

bool frm_fl_bulk_is_empty (struct frm_fl_bulk *self) {
  return self->size == 0;
  // for (int i = 0; i < self->size; i++)
  //   if (self->cursors[i] == 0)
  //     return false;
  // return true;
}

bool frm_fl_is_empty (struct frm_fl *self) {
  if (self->head == NULL)
    return true;

  // `head` can not be empty. so we don't check the tail.
  // `head` must be removed when it emptied.
  return frm_fl_bulk_is_empty (self->head);
}

static inline void _frm_fl_bulk_remove_frame(struct frm_fl_bulk *self,
    int index) {
  self->cursors[index] = -1;
  // we don't need to assign frame pointer's value to NULL since
  // we are always checking slot's existance from the cursor's
  // value. we just need to dereference it.
  frm_frame_unref(self->frames[index]);
  self->size--;
}

void frm_fl_bulk_move_cursors (struct frm_fl_bulk *self, int *move) {
  assert (*move > 0);
  for (int i = 0; *move > 0 && i < self->lastidx; i++) {
    int cursor = self->cursors[i];
    struct frm_frame *fr = self->frames[i];
    if (cursor == -1) // do not exits
      continue;

    /// needed move to complete the frame
    int needed = frm_frame_totlen(fr) - cursor;
    int dec = frm_min(*move, needed);

    cursor += dec;
    *move -= dec;

    if (cursor == frm_frame_totlen(fr))
      _frm_fl_bulk_remove_frame(self, i);
    else
      self->cursors[i] = cursor;
  }
}

void frm_fl_move_cursors (struct frm_fl *self, int move) {
  assert (!frm_fl_is_empty (self));
  while (move > 0) {
    struct frm_fl_bulk *bulk = self->head;
    if (!bulk) break;

    frm_fl_bulk_move_cursors(bulk, &move);

    if (frm_fl_bulk_is_empty (bulk)) {
      self->head = self->head->next;
      /// if no `bulk_free` cb provided, free the bulk.
      /// if `bulk_free` cb provided, reset the bulkd and
      /// pass it to the that.
      frm_fl_bulk_free (bulk);
    }
  }
  assert(move == 0);
}

void frm_fl_bulk_fill_iovs (struct frm_fl_bulk *self, struct iovec *iovs, int iovcnt, int *retiovcnt) {
  int ret = 0;

  for (int i = 0; iovcnt > 0 && i < self->lastidx; i++) {
    int cursor = self->cursors[i];
    struct frm_frame *fr = self->frames[i];
    if (cursor == -1) // do not exits
      continue;

    if (cursor < 4) {
      iovs[ret].iov_base = (char *)&fr->size + cursor;
      iovs[ret].iov_len = 4 - cursor;
      ret++;
      iovcnt--;
      cursor = 0; // hack. by setting cursor to the zero, we can easily calculate the
                  // frame i/o vector's buffer and size.
      if (iovcnt == 0)
        break;
    } else        //             ^
     cursor -= 4; // look above |||

    iovs[ret].iov_base = fr->buf + cursor;
    iovs[ret].iov_len = fr->size - cursor;
    ret++;
    iovcnt--;
  }

  *retiovcnt = ret;
}

void frm_fl_fill_iovs (struct frm_fl *self, struct iovec *iovs, int iovcnt, int *retiovcnt) {
  int ret = 0;
  struct frm_fl_bulk *bulk = self->head;
  while (iovcnt > 0) {
    if (!bulk) break;
    int bulkret;
    frm_fl_bulk_fill_iovs (bulk, &iovs[ret], iovcnt, &bulkret);
    ret += bulkret;
    iovcnt -= bulkret;
    bulk = bulk->next;
  }
  *retiovcnt = ret;
}
