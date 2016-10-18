#include "frame-list.h"

struct frm_parser {
  int curr_cursor;
  int embed_allow;
  struct frm_frame curr_frame;
  struct frm_fl in_frames;
};

int frm_parser_init (struct frm_parser *self, int embed_allow);
int frm_parser_parse (struct frm_parser *self, struct frm_cbuf *cbuf, int *bufsz);
bool frm_parser_has_frame (struct frm_parser *self);
