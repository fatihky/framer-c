/*
  # Framer Frame List
  # description
    frame list holds frame pointers and cursors of them. this aproach allows us
  to use the same frame at everywhere with small overhead.
    frame list both can be used for filling empty frame objects and writing them
  out.

  # reusable
  `free_cb` allows to use item group again and again. when it is done (every
  frame written), add it to your free list again or so. If `free_cb` is NULL,
  item group will be free'd.
*/

#include <sys/uio.h>
#include <stdbool.h>

#include "frame.h"

#ifndef FRM_FL_INCLUDED
#define FRM_FL_INCLUDED

/**
 * Cursor values:
 * 0: frame in the list
 * -1: frame not in the list(removed). empty slot
 * >0: frame not completely read yet(or not written).
 */

struct frm_fl_bulk {
  // private
  struct frm_frame **frames;
  int *cursors;
  int lastidx;
  int size;
  int capacity;
  struct frm_fl_bulk *next;
};

struct frm_fl {
  struct frm_fl_bulk *head;
  struct frm_fl_bulk *tail;
  // ro
  int capacity;
  // rw
  void *data;
  void (*free_cb) (struct frm_fl *self);
};

/**
 * Create new `struct frm_fl_bulk` instance.
 * @param {int} capacity Capacity of the frame list bulk
 * @returns `struct frm_fl_bulk *` on success, NULL on error and sets `errno`.
 */

struct frm_fl_bulk *frm_fl_bulk_new (int capacity);

/**
 * Add new frame to the bulk
 * This function increases frame's reference count so it won't be free'd before
 * written.
 * @param {struct frm_frame} frame to add
 * @returns true on success, false if bulk is full
 */

bool frm_fl_bulk_add (struct frm_fl_bulk *self, struct frm_frame *fr);

/**
 * Check if list bulk is empty
 * @returns true if list is empty
 */

bool frm_fl_bulk_is_empty (struct frm_fl_bulk *self);

/**
 * Frees all resources of the bulk
 */

void frm_fl_bulk_term (struct frm_fl_bulk *self);

/**
 * Frees the bulk
 * No need to call `frm_fl_bulk_term` seperately if you use
 * this function. This calls it if no `free_cb` is provided;
 */

void frm_fl_bulk_free (struct frm_fl_bulk *self);

/**
 * Move cursors by `move` value.
 * This moves cursors and derefences completed frames and resets itself
 * with `frm_fl_bulk_reset`(capacity argument will be current capacity).
 * @param {int *} move Move amount(aka `written bytes count`)
 */

void frm_fl_bulk_move_cursors (struct frm_fl_bulk *self, int *move);

/**
 * Create new `struct frm_fl` instance.
 * @see `frm_fl_init`
 * @param {int} capacity Capacity of the frame list
 * @returns `struct frm_fl *` on success, NULL on error and sets `errno`.
 */

struct frm_fl *frm_fl_new (int capacity);

/**
 * Initialize frame list.
 * Sets `free_cb` and `data` to NULL. If you want to use them, assign them after
 * this function.
 * @param {int} capacity Capacity of the frame list
 * @returns 0 on success, error code on error(ENOMEM probably)
 */

int frm_fl_init (struct frm_fl *self, int capacity);

/**
 * Reset the frame list. Allocates memory for `frames` and `cursors`
 * if current `capacity` is smaller than `@param capacity`.
 * @param {int} capacity Capacity of the frame list
 * @returns 0 on success, error code on error(ENOMEM probably)
 */

int frm_fl_reset (struct frm_fl *self, int capacity);

/**
 * Terminate list's resources.
 * This function DON'T call `free_cb`.
 * This function dereferences all of the frames inside the list.
 * Sets `capacity` to zero. Use 'frm_fl_reset' to re-init again.
 */

void frm_fl_term (struct frm_fl *self);

/**
 * Frees the list.
 * This functions calls `free callback`(free_cb) if provided.
 */

void frm_fl_free (struct frm_fl *self);

/**
 * Add new frame to the list
 * This function increases frame's reference count so it won't be free'd before
 * written.
 * @param {struct frm_frame} frame to add
 * @returns true on success, false if list is full
 */

bool frm_fl_add (struct frm_fl *self, struct frm_frame *fr);

/**
 * Check if list is empty
 * @returns true if list is empty
 */

bool frm_fl_is_empty (struct frm_fl *self);

/**
 * Move cursors by `move` value.
 * This moves cursors and derefences completed frames and resets itself
 * with `frm_fl_reset`(capacity arguments will be current capacity).
 * @param {int} move Move amount(aka `written bytes count`)
 */

void frm_fl_move_cursors (struct frm_fl *self, int move);

/**
 * Pop first element from the list
 * @returns {struct frm_frame *} if list isn't empty, NULL otherwise
 */

struct frm_frame *frm_fl_is_pop (struct frm_fl *self);

void frm_fl_fill_iovs (struct frm_fl *self, struct iovec *iovs, int iovcnt, int *retiovcnt);

#endif
