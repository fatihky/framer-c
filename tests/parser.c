#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "describe/describe.h"
#include "parser.h"

int main(int argc, char *argv[]) {
  struct frm_parser parser;
  int rc;
  int bufsz;

  describe("parser") {
    /// parse a frame
    it ("parse a frame") {
      rc = frm_parser_init (&parser, 0);
      assert (rc == 0);
      int len = 5;
      struct frm_cbuf *cbuf = frm_cbuf_new (9);
      memcpy(cbuf->buf, &len, 4);
      memcpy(cbuf->buf + 4, "fatih", 5);
      bufsz = 9;
      rc = frm_parser_parse (&parser, cbuf, &bufsz);
      assert (frm_parser_has_frame (&parser));
      assert (rc == 1);
    }
  
    /// parse two or more frames
    it ("parse two or more frames") {
      rc = frm_parser_init (&parser, 0);
      assert (rc == 0);
      int len = 5;
      struct frm_cbuf *cbuf = frm_cbuf_new (18);
      memcpy(cbuf->buf, &len, 4);
      memcpy(cbuf->buf + 4, "fatih", 5);
      memcpy(cbuf->buf + 9, &len, 4);
      memcpy(cbuf->buf + 13, "fatih", 5);
      bufsz = 18;
      rc = frm_parser_parse (&parser, cbuf, &bufsz);
      assert (frm_parser_has_frame (&parser));
      assert (rc == 2);
    }
  
    /// parse two or more frames
    it ("allow frame embbedding") {
      rc = frm_parser_init (&parser, 1); // embed_allow
      assert (rc == 0);
      int len = 5;
      struct frm_cbuf *cbuf = frm_cbuf_new (9);
      memcpy(cbuf->buf, &len, 4);
      memcpy(cbuf->buf + 4, "fatih", 5);
      bufsz = 9;
      rc = frm_parser_parse (&parser, cbuf, &bufsz);
      assert (frm_parser_has_frame (&parser));
      assert (rc == 1);
    }
  
    /// parse two or more frames
    it ("parse partial buffers") {
      rc = frm_parser_init (&parser, 0);
      assert (rc == 0);
      int len = 5;
      struct frm_cbuf *cbuf = frm_cbuf_new (18);
      memcpy(cbuf->buf, &len, 4);
      memcpy(cbuf->buf + 4, "fatih", 5);
      memcpy(cbuf->buf + 9, &len, 4);
      memcpy(cbuf->buf + 13, "fatih", 5);
      // send two bytes, not enought for length. but should work
      bufsz = 2;
      rc = frm_parser_parse (&parser, cbuf, &bufsz);
      // send one more byte. now parser have 3 bytes for the length
      cbuf->buf += 2;
      bufsz = 2;
      rc = frm_parser_parse (&parser, cbuf, &bufsz);
      // send one more byte. now parser have 4 bytes for the length
      // it should be parsed the frame size
      cbuf->buf += 1;
      bufsz = 1;
      rc = frm_parser_parse (&parser, cbuf, &bufsz);
      // send two more bytes. now parser have 2 bytes for the frame content
      cbuf->buf += 1;
      bufsz = 2;
      rc = frm_parser_parse (&parser, cbuf, &bufsz);
      // send three more bytes. now parser have 5 bytes for the frame content
      // it should be parsed the frame content
      cbuf->buf += 2;
      bufsz = 3;
      rc = frm_parser_parse (&parser, cbuf, &bufsz);
      assert (frm_parser_has_frame (&parser));
    }
  }

  return assert_failures();
}
