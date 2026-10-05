#include "callback_owner_fixture.h"

int main() {
  AlphaMovie movie;
  Win *window = &movie;
  if (window->on_key_click(7, 9) != 1 || movie.release_calls != 1) return 1;

  BaseWin base;
  window = &base;
  window->vslot_49(3, 14);
  if (base.scroll_calls != 1 || base.scroll_code != 3 || base.scroll_pos != 14) return 2;

  Credits credits;
  window = &credits;
  if (window->on_key_click(2, 5) != 1) return 3;
  window->on_left_click_event(10, 11);
  if (credits.release_calls != 2) return 4;

  NewTechWin tech;
  window = &tech;
  window->on_button_clicked(8);
  if (tech.release_calls != 1) return 5;

  ProdPicker picker;
  window = &picker;
  window->vslot_49(4, 21);
  if (picker.scroll_calls != 1 || picker.scroll_code != 4 || picker.scroll_pos != 21) return 6;

  ReplayWin replay;
  window = &replay;
  window->on_left_click_event(1, 2);
  window->vslot_22(3, 4);
  window->on_button_clicked(5);
  if (window->on_key_click(6, 7) != 1 || replay.release_calls != 4) return 7;

  Gamma gamma;
  window = &gamma;
  window->vslot_49(6, 35);
  if (gamma.scroll_calls != 1 || gamma.scroll_code != 6 || gamma.scroll_pos != 35) return 8;

  BaseButton button;
  window = &button;
  window->on_dialog_focus(4);
  if (button.redraw_calls != 1 || button.release_calls != 0) return 9;
  return 0;
}
