// Host-only interface projection for callback owner body controls.
// This is not a product header and contains no proposed storage.
#pragma once

class Win {
 public:
  Win() : release_calls(0), redraw_calls(0), scroll_calls(0), scroll_code(0), scroll_pos(0) {}
  virtual int on_key_click(int, int) { return 0; }
  virtual void on_left_click(int, int) {}
  virtual void on_right_click(int, int) {}
  virtual void on_button_clicked(int) {}
  virtual void vslot_48(int, int) {}
  virtual void vslot_49(int, int) {}
  virtual void on_dialog_focus(int) {}
  virtual void release_modal() { ++release_calls; }
  virtual void redraw() { ++redraw_calls; }

  int release_calls;
  int redraw_calls;
  int scroll_calls;
  int scroll_code;
  int scroll_pos;
};

class AlphaMovie : public Win {
 public:
  int on_key_click(int, int);
};
class BaseWin : public Win {
 public:
  void vslot_48(int code, int pos) {
    ++scroll_calls; scroll_code = code; scroll_pos = pos;
  }
  void vslot_49(int, int);
};
class Credits : public Win {
 public:
  int on_key_click(int, int);
  void on_left_click(int, int);
};
class NewTechWin : public Win {
 public:
  void on_button_clicked(int);
};
class ProdPicker : public Win {
 public:
  void vslot_48(int code, int pos) {
    ++scroll_calls; scroll_code = code; scroll_pos = pos;
  }
  void vslot_49(int, int);
};
class ReplayWin : public Win {
 public:
  int on_key_click(int, int);
  void on_left_click(int, int);
  void on_right_click(int, int);
  void on_button_clicked(int);
};
class Gamma : public Win {
 public:
  void vslot_48(int code, int pos) {
    ++scroll_calls; scroll_code = code; scroll_pos = pos;
  }
  void vslot_49(int, int);
};
class BaseButton : public Win {
 public:
  void on_dialog_focus(int);
};
