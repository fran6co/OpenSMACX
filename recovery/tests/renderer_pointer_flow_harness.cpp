// C++98 actual-source flow fixture. The control injects FlatButton ctor/close
// and BasePop::apply_button_config bodies extracted from product owners.
#include <cassert>
#include <stdint.h>
#include <cstring>
#define nullptr 0

struct Sprite { int identity; };
Sprite *FlatButtonDefaultsTable[27];
struct Time {
  int closes;
  Time() : closes(0) {}
  void close() { ++closes; }
};
struct BaseButton {
  Time time1_;
  int base_closes;
  BaseButton() : base_closes(0) {}
  void close() { ++base_closes; }
};
struct FlatButton : BaseButton {
  uint32_t field_A18_, field_A14_, field_AB8_;
  uint32_t field_ABC_[3];
  Sprite *field_AD4_[3];
  Sprite *border_styles_[3];
  Sprite *config[27];
  FlatButton();
  void close();
  void set_slice_icons(Sprite *, Sprite *, Sprite *, Sprite *, Sprite *, Sprite *,
                       Sprite *, Sprite *, Sprite *);
};
Sprite *BasePopDefaultBlock31B8[27];
struct BasePop {
  Sprite *default_block_[27];
  void apply_button_config(FlatButton *button);
};

// ACTUAL_FLATBUTTON_CTOR
// ACTUAL_FLATBUTTON_CLOSE
// ACTUAL_FLATBUTTON_SLICE_SETTER
// ACTUAL_BASEPOP_APPLY_CONFIG

int main() {
  Sprite objects[27];
  for (int i = 0; i < 27; ++i) {
    objects[i].identity = i;
    FlatButtonDefaultsTable[i] = &objects[i];
  }
  FlatButton button;
  for (int i = 0; i < 27; ++i) assert(button.config[i] == &objects[i]);

  for (int i = 0; i < 27; ++i) {
    button.config[i] = 0;
    FlatButtonDefaultsTable[i] = &objects[26 - i];
  }
  button.close();
  assert(button.time1_.closes == 1 && button.base_closes == 1);
  assert(button.field_A18_ == 0 && button.field_A14_ == 0);
  assert(button.field_AB8_ == 0xFFFFFFFFU);
  for (int i = 0; i < 27; ++i) assert(button.config[i] == &objects[26 - i]);

  button.set_slice_icons(&objects[0], &objects[1], &objects[2],
                         &objects[3], &objects[4], &objects[5],
                         &objects[6], &objects[7], &objects[8]);
  assert(button.config[12] == &objects[0] && button.config[13] == &objects[1]
         && button.config[14] == &objects[2]);
  assert(button.config[6] == &objects[3] && button.config[7] == &objects[4]
         && button.config[8] == &objects[5]);
  assert(button.config[15] == &objects[6] && button.config[16] == &objects[7]
         && button.config[17] == &objects[8]);

  BasePop popup;
  for (int i = 0; i < 27; ++i) {
    popup.default_block_[i] = &objects[(i * 7) % 27];
    button.config[i] = 0;
  }
  popup.apply_button_config(&button);
  for (int i = 0; i < 27; ++i) {
    assert(button.config[i] == popup.default_block_[i]);
    assert(button.config[i] == &objects[(i * 7) % 27]);
  }
  return 0;
}
