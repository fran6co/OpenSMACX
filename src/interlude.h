#pragma once
#include "graphicwin.h"
#include "stringstruct.h"

class Interlude : public GraphicWin {
 public:
  Interlude();
  virtual ~Interlude();
  void exec(char *section);
  void close();
  virtual void on_redraw();
  virtual void on_left_down(int x, int y);
  virtual int on_key_down(int key) override;
 private:
  // Original construction and redraw access these real embedded subobjects.
  // RECOVERY_MEMBER: v1 Interlude text_rect_ 0xA14 0x0045F5C0
  RECT text_rect_;
  // RECOVERY_MEMBER: v2 Interlude captions_ StringList 0xA24 0x0045F200
  StringList captions_;
  // RECOVERY_MEMBER: v2 Interlude art_ Buffer 0xA54 0x0045F200
  Buffer art_;
};

void __cdecl interlude(int id, char *text, int mode, int ending);
