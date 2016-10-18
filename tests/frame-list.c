#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "describe/describe.h"
#include "frame-list.h"

int main(int argc, char *argv[]) {
  bool res;
  struct frm_fl *fl;
  struct frm_frame frame;
  struct iovec iovs[4];
  int retiovcnt;

  // little innocent hack
  frame.size = 5;
  frame.ref = 10;

  describe("frame list") {

    it ("can write a frame") {
      fl = frm_fl_new (2);
      assert (fl != NULL);
      res = frm_fl_add (fl, &frame);
      assert (res);
      assert (!frm_fl_is_empty (fl));
      frm_fl_move_cursors (fl, 9);
      assert (frm_fl_is_empty (fl));
    }

    it ("can write a frame in multiple steps") {
      fl = frm_fl_new (2);
      assert (fl != NULL);
      res = frm_fl_add (fl, &frame);
      assert (res);
      assert (!frm_fl_is_empty (fl));
      frm_fl_move_cursors (fl, 3);
      frm_fl_move_cursors (fl, 3);
      frm_fl_move_cursors (fl, 2);
      frm_fl_move_cursors (fl, 1);
      assert (frm_fl_is_empty (fl));
    }

    it ("can write two frames in multiple steps") {
      fl = frm_fl_new (2);
      assert (fl != NULL);
      res = frm_fl_add (fl, &frame);
      assert (res);
      res = frm_fl_add (fl, &frame);
      assert (res);
      assert (!frm_fl_is_empty (fl));
      frm_fl_move_cursors (fl, 3);
      frm_fl_move_cursors (fl, 3);
      frm_fl_move_cursors (fl, 2);
      frm_fl_move_cursors (fl, 1);
      frm_fl_move_cursors (fl, 3);
      frm_fl_move_cursors (fl, 4);
      frm_fl_move_cursors (fl, 2);
      assert (frm_fl_is_empty (fl));
    }

    it ("can fill an iov") {
      fl = frm_fl_new (2);
      assert (fl != NULL);
      res = frm_fl_add (fl, &frame);
      assert (res);
      assert (!frm_fl_is_empty (fl));
      frm_fl_fill_iovs(fl, iovs, 4, &retiovcnt);
      assert (retiovcnt == 2);
      assert (iovs[0].iov_len == 4);
      assert (iovs[0].iov_base == &frame.size);
      assert (iovs[1].iov_len == 5);
      assert (iovs[1].iov_base == frame.buf);
    }

    it ("can fill an iov with two frames") {
      fl = frm_fl_new (2);
      assert (fl != NULL);
      res = frm_fl_add (fl, &frame);
      assert (res);
      res = frm_fl_add (fl, &frame);
      assert (res);
      assert (!frm_fl_is_empty (fl));
      frm_fl_fill_iovs(fl, iovs, 4, &retiovcnt);
      assert (retiovcnt == 4);
      assert (iovs[0].iov_len == 4);
      assert (iovs[0].iov_base == &frame.size);
      assert (iovs[1].iov_len == 5);
      assert (iovs[1].iov_base == frame.buf);
      assert (iovs[2].iov_len == 4);
      assert (iovs[2].iov_base == &frame.size);
      assert (iovs[3].iov_len == 5);
      assert (iovs[3].iov_base == frame.buf);
    }

    it ("can fill iovs after writes") {
      fl = frm_fl_new (2);
      assert (fl != NULL);
      res = frm_fl_add (fl, &frame);
      assert (res);
      res = frm_fl_add (fl, &frame);
      assert (res);
      assert (!frm_fl_is_empty (fl));
      frm_fl_fill_iovs(fl, iovs, 4, &retiovcnt);
      assert (retiovcnt == 4);

      // test the `size`
      assert (iovs[0].iov_len == 4);
      assert (iovs[0].iov_base == &frame.size);
      // say list we written some data
      frm_fl_move_cursors (fl, 2);
      // fill `iovs` again
      frm_fl_fill_iovs(fl, iovs, 4, &retiovcnt);
      // check if it filled correct amount of iovs
      assert (retiovcnt == 4);
      // check if it returned correct variables
      assert (iovs[0].iov_len == 2);
      assert (iovs[0].iov_base == (char *)&frame.size + 2);

      // lets move cursor and check if it will delete or not delete firs iov
      frm_fl_move_cursors (fl, 2);
      // fill `iovs`
      frm_fl_fill_iovs(fl, iovs, 4, &retiovcnt);
      // check if it filled correct amount of iovs
      assert (retiovcnt == 3);

      assert (iovs[0].iov_len == 5);
      assert (iovs[0].iov_base == frame.buf);
      assert (iovs[1].iov_len == 4);
      assert (iovs[1].iov_base == &frame.size);
      assert (iovs[2].iov_len == 5);
      assert (iovs[2].iov_base == frame.buf);
    }

  }

  return assert_failures();
}