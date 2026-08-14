// ORIGINAL: 0x005DB450
// name      sub_5db450
// size      289 bytes
// spans     0x005DB450-0x005DB571
// prototype int (__stdcall sub_5DB450)(int8*, CHAR String)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DD020 0x006453E0
// RULED-OUT: the fallback stack-only signature has the wrong arity - the
//            real function reads ecx as a Buffer receiver (rect1_ at 0x20,
//            font1_ at 0x52c, both matching Buffer's already-catalogued
//            layout), then a text pointer and a RECT pointer on the stack.
//            It draws each character of the text into a one-byte local
//            buffer and calls write_cent_l once per character, advancing y
//            by the font's line height each time - a vertical column of
//            single-character lines, not a normal horizontal string write.
//            Landed as a free stdcall function taking the receiver
//            explicitly, rather than a Buffer member, because a new
//            top-level type declared here to carry the method would
//            truncate the region this file's own extractor reads at that
//            type's own closing brace, before the real definition.

extern "C" int __stdcall sub_5db450(Buffer *self, char *text, RECT *rect) {
  if (rect == 0) {
    return 0;
  }
  RECT saved = self->rect1_;
  self->set_clip(rect);
  size_t len = strlen(text);

  if (self->font1_ == 0) {
    self->font1_ = (Font *)*g_009bb484;
  }
  int *f = (int *)self->font1_;
  int lineHeight;
  if (*f < 0) {
    lineHeight = f[3];
  } else {
    lineHeight = f[4] + *f;
  }
  int bottom = rect->bottom;
  int top = rect->top;
  int total = lineHeight * (int)len;
  for (;;) {
    if (self->font1_ == 0) {
      self->font1_ = (Font *)*g_009bb484;
    }
    f = (int *)self->font1_;
    if (lineHeight <= f[4] || total <= bottom - top) {
      break;
    }
    lineHeight = lineHeight - 1;
    total = total - (int)len;
  }

  int y = rect->top;
  int x = rect->left;
  int width = rect->right - rect->left;
  char ch = *text;
  while (ch != 0) {
    ch = *text;
    text = text + 1;
    char onechar[2];
    onechar[0] = ch;
    onechar[1] = 0;
    size_t l = strlen(onechar);
    self->write_cent_l(onechar, x, y, width, (int)l);
    y = y + lineHeight;
    ch = *text;
  }

  self->set_clip(&saved);
  return y;
}
