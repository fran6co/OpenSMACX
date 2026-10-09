/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// ORIGINAL: 0x005FA770 ??_GPullDown@@UAEPAXI@Z 0x005FA770-0x005FA78E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x005FA770-0x005FA78E", "Pinned class constructor ??0PullDown@@QAE@XZ at 0x005F8800 stores original class/view table VA 0x66ff40 at 0x005F8808 (mov dword ptr [esi], 0x66ff40); the current genuine owner vftable ??_7PullDown@@6BWin@@@ selects generated ??_GPullDown@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EPullDown@@UAEPAXI@Z (mode 2).", "Pinned ??_GPullDown@@UAEPAXI@Z at 0x005FA770 calls the source-mapped ??1PullDown@@QAE@XZ at 0x005F88A0, then tests byte ptr [esp + 8], 1; je 0x5fa788 bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/pulldown.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original PullDown scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GPullDown@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "general.h"
#include "win.h"
#include "pulldown.h"
#include "menu.h"
#include "checkbox.h"
#include "console.h"
#include "font.h"
#include <stdlib.h>
#include <ctype.h>

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005F8970 ?UNK1@PullDown@@QAEXXZ 0x005F8970-0x005F89C8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005f8970.cpp", "image:terranx_original.exe:0x005F8970", "Native row flags and old/new redraw asymmetry, callback modes and padding reloads; mnemonic metrics use Font internal_leading at+14. UNK1 resets members after clearing items and closing; outside hover forwards screen coordinates to Menu.", "recovery/tasks/interface-contract-report-closure.json four-method owner closure"], "reason": "Recover the bounded original in its genuine PullDown owner using actual members, owned callbacks and Buffer/GraphicWin behavior. Retain seed and historical obligations; this binding grants no current match or runtime claim."}
void PullDown::UNK1() {
  UNK4();
  close();
  selected_ = -1;
  field_F18_ = 0;
  poll_ = 0;
  item_count_ = 0;
  width_ = 0;
  menu_ = 0;
  visible_count_ = 0;
  field_F38_ = PullDownFieldF38Default;
  field_F3C_ = PullDownFieldF3CDefault;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FA6B0 ?on_mouse_move@PullDown@@QAEXHH@Z 0x005FA6B0-0x005FA736 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005fa6b0.cpp", "image:terranx_original.exe:0x005FA6B0", "Native row flags and old/new redraw asymmetry, callback modes and padding reloads; mnemonic metrics use Font internal_leading at+14. UNK1 resets members after clearing items and closing; outside hover forwards screen coordinates to Menu.", "recovery/tasks/interface-contract-report-closure.json four-method owner closure"], "reason": "Recover the bounded original in its genuine PullDown owner using actual members, owned callbacks and Buffer/GraphicWin behavior. Retain seed and historical obligations; this binding grants no current match or runtime claim."}
// symbol    ?on_mouse_move@PullDown@@UAEXHH@Z
void PullDown::on_mouse_move(int x, int y) {
  if (in_box(x, y, buffer_working_rect())) {
    mouse_move(x, y);
    return;
  }
  if (selected_ != -1) {
    selected_ = -1;
    redraw();
  }
  if (menu_ != 0) {
    client_to_screen(&x, &y);
    menu_->mouse_move(x, y);
  }
}

// Native body 0x005F9640, hosted by the PullDown slot-25 override selected in
// the constructor-installed 0x0066FF40 table. The retained seed mapping is
// transferred to this real owner by the source writer at coordinator import.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005F9640 ?on_key_click@PullDown@@QAEHHH@Z 0x005F9640-0x005F9840
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005f9640.cpp", "image:Menu6700B0/AlphaMenu66ED88/PullDown66FF40 constructor-selected primary tables; Buffer secondary subobject+444", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover this complete operation in its genuine Menu/PullDown owner, using native constructor-selected virtual dispatch, real members and shared source-owned dependencies; preserve original identity, retained seed and historical restoration obligations."}
// symbol    ?on_key_click@PullDown@@UAEHHH@Z
int PullDown::on_key_click(int key_input, int key) {
  const int old_selected = selected_;
  key &= 0xFFFBFFFF;

  switch (key) {
  case 0x1B:
    selected_ = -1;
    // Native Escape falls through to the same modal-release path as Enter.
    // fall through
  case 0x0D:
    release_modal();
    return 1;

  case 0x26:
  case 0x68: {
    const int next = old_selected > 0 ? old_selected - 1 : item_count_ - 1;
    selected_ = next;
    int index = next;
    if (index < 0) {
      selected_ = -1;
    } else {
      for (;;) {
        const uint32_t flags = items_[index].flags;
        if ((flags & 1u) != 0 && (flags & 4u) == 0) {
          selected_ = index;
          break;
        }
        --index;
        if (index < 0) {
          selected_ = -1;
          break;
        }
      }
    }
    MenuPullDownInputOrigin = 1;
    pulldown_update(old_selected, selected_);
    return 1;
  }

  case 0x28:
  case 0x62: {
    const int count = item_count_;
    const int next = old_selected == count - 1 ? 0 : old_selected + 1;
    selected_ = next;
    int index = next;
    if (index < count) {
      for (;;) {
        const uint32_t flags = items_[index].flags;
        if ((flags & 1u) != 0 && (flags & 4u) == 0) break;
        ++index;
        if (index >= count) break;
      }
    }
    selected_ = index >= count ? -1 : index;
    MenuPullDownInputOrigin = 1;
    pulldown_update(old_selected, selected_);
    return 1;
  }

  default:
    for (int index = 0; index < item_count_; ++index) {
      const uint32_t flags = items_[index].flags;
      char *mnemonic = items_[index].mnemonic;
      if ((flags & 1u) == 0 || (flags & 2u) != 0 || mnemonic == 0) continue;
      const int mnemonic_character = static_cast<signed char>(mnemonic[0]);
      if (mnemonic_character == tolower(key_input) ||
          mnemonic_character == toupper(key_input)) {
        selected_ = index;
        redraw();
        release_modal();
        return 1;
      }
    }
    if (win_parent_ != 0) {
      win_parent_->on_key_click(key_input, key);
    }
    return 0;
  }
}

// ORIGINAL: 0x005F9590 ?mouse_move@PullDown@@QAEXHH@Z 0x005F9590-0x005F9631
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5F9590..5F9631; prior mapping in src/stringstruct.cpp; seed git:117282ee:src/unrecovered/005f9590.cpp", "Items at+A18, flags+0C, count+F20, selected+F28; Font line advance and border drive visible row scan. Verbatim FILE wrapper becomes BODY mapping on the actual member, with identical original spans and no claim change.", "recovery/tasks/interface-contract-report-closure.json four-method ownership closure; src/pulldown.cpp pulldown_update replaces original-address call"], "reason": "Rehome the complete hover tracker into its genuine PullDown owner, replacing raw receiver offsets and the fixed executable callee with members and the recovered pulldown_update. Preserve original identity and seed provenance; no current matching or runtime claim."}
void PullDown::mouse_move(int, int y) {
  const int old_selected = selected_;
  const int row = (y - PullDownBorderPadding) / PullDownDefaultFont->line_advance();
  selected_ = row;

  int found;
  if (row < 0) {
    found = item_count_;
  } else {
    int visible_item = -1;
    found = 0;
    if (item_count_ > 0) {
      do {
        if ((items_[found].flags & 1u) != 0) {
          ++visible_item;
        }
        if (visible_item == selected_) {
          break;
        }
        ++found;
      } while (found < item_count_);
    }
  }

  if (found == item_count_) {
    selected_ = -1;
  } else {
    selected_ = found;
    if ((items_[found].flags & 4u) != 0) {
      selected_ = -1;
    }
  }
  pulldown_update(old_selected, selected_);
}

// ORIGINAL: 0x005F9F60 ?pulldown_update@PullDown@@QAEXHH@Z 0x005F9F60-0x005FA6AB
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005f9f60.cpp", "image:terranx_original.exe:0x005F9F60", "Native row flags and old/new redraw asymmetry, callback modes and padding reloads; mnemonic metrics use Font internal_leading at+14. UNK1 resets members after clearing items and closing; outside hover forwards screen coordinates to Menu.", "recovery/tasks/interface-contract-report-closure.json four-method owner closure"], "reason": "Recover the bounded original in its genuine PullDown owner using actual members, owned callbacks and Buffer/GraphicWin behavior. Retain seed and historical obligations; this binding grants no current match or runtime claim."}
// TRIED: mechanical Ghidra-shape transcription, with the four scalar "rect-ish" locals (left/top/right/bottom passed as &local_20) promoted to a real RECT so fill/write_l/write_right_l/ soft_update get valid contiguous storage instead of relying on adjacent-local layout; fill() moved from Buffer to GraphicWin (real receiver is `this`, not the Buffer at self+0x444). Original prologue is frameless (`sub esp,0x38; push ebx...`); not chased to byte-exact given size (1867 bytes).
void PullDown::pulldown_update(int old_selected, int new_selected) {
  Buffer *const buffer = static_cast<Buffer *>(this);
  const int pixel_width = buffer->pixel_width();
  const int row_height = buffer->text_line_height();
  const int border = PullDownBorderPadding;
  const int padding = PullDownTextPadding;
  const int text_left = padding + border;
  const int text_right = pixel_width - text_left;
  const int first_top = border;
  const int first_bottom = border + row_height;

  if (old_selected != -1) {
    int visible_before = 0;
    for (int index = 0; index < old_selected; ++index) {
      if ((items_[index].flags & 1u) != 0) {
        ++visible_before;
      }
    }

    const int observed_row_height = buffer->text_line_height();
    RECT row = {text_left,
                first_top + observed_row_height * visible_before,
                text_right,
                first_bottom + observed_row_height * visible_before};
    PullDownItem &item = items_[old_selected];
    const uint32_t flags = item.flags;
    char *const text = item.text;
    char *const right_text = item.right_text;
    char *const mnemonic = item.mnemonic;
    if ((flags & 1u) != 0) {
      if ((flags & 8u) != 0) {
        Sprite *const icon = CheckBoxDefault2;
        const int icon_y = ((row.bottom - icon->height()) - row.top) / 2 + row.top;
        icon->draw(buffer, icon->transparent_index(), row.left, icon_y, 1, 1);
        row.left += CheckBoxDefault2->width();
      }

      if ((flags & 4u) != 0) {
        const int first_border = PullDownBorderPadding;
        const int first_width = buffer->pixel_width() - 2 * first_border;
        const int first_color = PullDownDefaultDisabledTextColor;
        const int first_y = buffer->text_line_height() / 2 + row.top;
        const int first_left = PullDownBorderPadding;
        buffer->hline(first_left, first_left + first_width, first_y, first_color);
        const int second_border = PullDownBorderPadding;
        const int second_width = buffer->pixel_width() - 2 * second_border;
        const int second_color = PullDownDefaultSelectedTextShadow;
        const int second_y = buffer->text_line_height() / 2 + row.top + 1;
        const int second_left = PullDownBorderPadding;
        buffer->hline(second_left, second_left + second_width, second_y, second_color);
      } else {
        int text_color;
        int text_shadow;
        if (old_selected == selected_) {
          if (field_F38_ == 0) {
            row.left -= PullDownTextPadding;
            row.right += PullDownTextPadding;
            fill(&row, PullDownDefaultBackground);
            row.left += PullDownTextPadding;
            row.right -= PullDownTextPadding;
          } else {
            field_F38_(this, 1, &row);
          }
          text_color = PullDownDefaultTextColor;
          text_shadow = PullDownDefaultTextShadow;
        } else {
          if (field_F38_ == 0) {
            row.left -= PullDownTextPadding;
            row.right += PullDownTextPadding;
            fill(&row, PullDownDefaultTextColor);
            row.left += PullDownTextPadding;
            row.right -= PullDownTextPadding;
          } else {
            field_F38_(this, 0, &row);
          }
          text_color = PullDownDefaultSelectedTextColor;
          text_shadow = PullDownDefaultDisabledTextShadow;
        }
        if ((flags & 2u) == 0) {
          text_color = text_shadow;
        }
        buffer->set_text_color(text_color, -1, 1, 1);

        char *const rendered_text = item.text;
        if (rendered_text != 0) {
          buffer->write_l(rendered_text, &row, static_cast<int>(strlen(rendered_text)));
        }
        if (mnemonic != 0) {
          const int underline_bottom = row.bottom;
          const int underline_height = row.bottom - row.top;
          const int underline_color = buffer->foreground_color();
          const int leading = PullDownDefaultFont->internal_leading();
          const int letter_width = buffer->text_width(mnemonic, 1);
          const int center_height = buffer->text_line_height();
          const int underline_y =
              underline_bottom - ((underline_height - center_height) / 2)
              - leading;
          const int prefix_width = buffer->text_width(
              text, static_cast<int>(mnemonic - text));
          buffer->hline(row.left + prefix_width,
                        row.left + prefix_width + letter_width,
                        underline_y, underline_color);
        }
        if (right_text != 0) {
          buffer->write_right_l(right_text, &row,
                                static_cast<int>(strlen(right_text)));
        }
      }
      row.left -= PullDownTextPadding;
      row.right += PullDownTextPadding;
      soft_update(&row);
    }
  }

  if (new_selected != -1 && new_selected != old_selected) {
    int visible_before = 0;
    for (int index = 0; index < new_selected; ++index) {
      if ((items_[index].flags & 1u) != 0) {
        ++visible_before;
      }
    }

    const int observed_row_height = buffer->text_line_height();
    RECT row = {text_left,
                first_top + observed_row_height * visible_before,
                text_right,
                first_bottom + observed_row_height * visible_before};
    PullDownItem &item = items_[new_selected];
    const uint32_t flags = item.flags;
    char *const text = item.text;
    char *const right_text = item.right_text;
    char *const mnemonic = item.mnemonic;
    if ((flags & 1u) != 0) {
      if ((flags & 8u) != 0) {
        Sprite *const icon = CheckBoxDefault2;
        const int icon_y = ((row.bottom - icon->height()) - row.top) / 2 + row.top;
        icon->draw(buffer, icon->transparent_index(), row.left, icon_y, 1, 1);
        row.left += CheckBoxDefault2->width();
      }

      if ((flags & 4u) != 0) {
        const int first_border = PullDownBorderPadding;
        const int first_width = buffer->pixel_width() - 2 * first_border;
        const int first_color = PullDownDefaultDisabledTextColor;
        const int first_y = buffer->text_line_height() / 2 + row.top;
        const int first_left = PullDownBorderPadding;
        buffer->hline(first_left, first_left + first_width, first_y, first_color);
        const int second_border = PullDownBorderPadding;
        const int second_width = buffer->pixel_width() - 2 * second_border;
        const int second_color = PullDownDefaultSelectedTextShadow;
        const int second_y = buffer->text_line_height() / 2 + row.top + 1;
        const int second_left = PullDownBorderPadding;
        buffer->hline(second_left, second_left + second_width, second_y, second_color);
      } else {
        int text_color;
        int text_shadow;
        if (new_selected == selected_) {
          if (field_F38_ == 0) {
            row.left -= PullDownTextPadding;
            row.right += PullDownTextPadding;
            fill(&row, PullDownDefaultBackground);
            row.left += PullDownTextPadding;
            row.right -= PullDownTextPadding;
          } else {
            field_F38_(this, 1, &row);
          }
          text_color = PullDownDefaultTextColor;
          text_shadow = PullDownDefaultTextShadow;
        } else {
          if (field_F38_ == 0) {
            fill(&row, PullDownDefaultTextColor);
          } else {
            field_F38_(this, 0, &row);
          }
          text_color = PullDownDefaultSelectedTextColor;
          text_shadow = PullDownDefaultDisabledTextShadow;
        }
        if ((flags & 2u) == 0) {
          text_color = text_shadow;
        }
        buffer->set_text_color(text_color, -1, 1, 1);

        char *const rendered_text = item.text;
        if (rendered_text != 0) {
          buffer->write_l(rendered_text, &row, static_cast<int>(strlen(rendered_text)));
        }
        if (mnemonic != 0) {
          const int underline_bottom = row.bottom;
          const int underline_height = row.bottom - row.top;
          const int underline_color = buffer->foreground_color();
          const int leading = PullDownDefaultFont->internal_leading();
          const int letter_width = buffer->text_width(mnemonic, 1);
          const int center_height = buffer->text_height();
          const int underline_y =
              underline_bottom - ((underline_height - center_height) / 2)
              - leading;
          const int prefix_width = buffer->text_width(
              text, static_cast<int>(mnemonic - text));
          buffer->hline(row.left + prefix_width,
                        row.left + prefix_width + letter_width,
                        underline_y, underline_color);
        }
        if (right_text != 0) {
          buffer->write_right_l(right_text, &row,
                                static_cast<int>(strlen(right_text)));
        }
      }

      row.left -= PullDownTextPadding;
      row.right += PullDownTextPadding;
      soft_update(&row);
    }
  }

  do_all_draws();
}

namespace {

static const int ItemNotFound = 11;

__forceinline int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits), "PullDown requires 32-bit int");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

}  // namespace

/*
Purpose: Hide the first item with the requested ID.
// ORIGINAL: 0x005F8CB0 ?hide_item@PullDown@@QAEHH@Z 0x005F8CB0-0x005F8D1F BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      111 bytes
// prototype int (__thiscall ?hide_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::hide_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    if (items_[index].flags & 1U) {
        items_[index].flags &= ~1U;
        visible_count_ = int_from_bits(
            static_cast<uint32_t>(visible_count_) - 1U);
    }
    dirty_ = 1;
    return 0;
}

/*
Purpose: Show the first item with the requested ID.
// ORIGINAL: 0x005F8D20 ?show_item@PullDown@@QAEHH@Z 0x005F8D20-0x005F8D8F BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      111 bytes
// prototype int (__thiscall ?show_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::show_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    if (!(items_[index].flags & 1U)) {
        items_[index].flags |= 1U;
        visible_count_ = int_from_bits(
            static_cast<uint32_t>(visible_count_) + 1U);
    }
    dirty_ = 1;
    return 0;
}

/*
Purpose: Disable the first item with the requested ID.
// ORIGINAL: 0x005F8D90 ?disable_item@PullDown@@QAEHH@Z 0x005F8D90-0x005F8DEF BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      95 bytes
// prototype int (__thiscall ?disable_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::disable_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    items_[index].flags |= 2U;
    dirty_ = 1;
    return 0;
}

/*
Purpose: Enable the first item with the requested ID.
// ORIGINAL: 0x005F8DF0 ?enable_item@PullDown@@QAEHH@Z 0x005F8DF0-0x005F8E4F BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      95 bytes
// prototype int (__thiscall ?enable_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::enable_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    items_[index].flags &= ~2U;
    dirty_ = 1;
    return 0;
}

/*
Purpose: Check the first item with the requested ID.
// ORIGINAL: 0x005F9040 ?check_item@PullDown@@QAEHH@Z 0x005F9040-0x005F909F BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      95 bytes
// prototype int (__thiscall ?check_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::check_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    items_[index].flags |= 8U;
    dirty_ = 1;
    return 0;
}

/*
Purpose: Uncheck the first item with the requested ID.
// ORIGINAL: 0x005F90A0 ?uncheck_item@PullDown@@QAEHH@Z 0x005F90A0-0x005F90FF BYTE_EXACT
// LEVER: find_item moved to pulldown.h (inline, no ORIGINAL marker) and made index-returning: image recomputes the item address from the loop index at each field access instead of caching a pointer.
// size      95 bytes
// prototype int (__thiscall ?uncheck_item@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::uncheck_item(int id) {
    int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }
    items_[index].flags &= ~8U;
    dirty_ = 1;
    return 0;
}


/*
Purpose: Append an item, splitting its text at the first '|' into a left half
         and a right-aligned half, and isolating the mnemonic after '&'.
         A full table answers 1, a duplicate id answers 1, a missing text or a
         -1 id answers 3, an occupied slot answers 0xC, and an allocation
         failure answers 4.
// ORIGINAL: 0x005F89D0 ?add_item@PullDown@@QAEHHPAD@Z 0x005F89D0-0x005F8BDF BYTE_EXACT
// LEVER: items_[slot].field with `const int slot = new_count - 1`, NOT a
//          `PullDownItem *slot = &items_[slot]` pointer. The image keeps
//          `this + 20*(new_count-1)` in esi and carries items_'s own 0xa18 in
//          every displacement (`mov [esi+0xa20],ebp` for .id, `[esi+0xa18]`
//          for .text); a slot pointer emits `[esi+8]` / `[esi+0x10]` instead
//          and diverges from the very first store. Same shape as the
//          BYTE_EXACT add_separator beside it. `&items_[slot].mnemonic` is
//          the one address the image does fold from scratch
//          (`lea ecx,[edi+edi*4+0x285]; lea ebp,[ebx+ecx*4]`), which is what
//          spelling the mnemonic through items_[slot] twice lets VC6 CSE.
// TRIED: the duplicate scan spelled out as a local loop rather than the
//          shared find_item; the forwarder's Ghidra-derived stack-slot
//          aliasing reading. find_item is both shorter and the family's
//          established helper - and `if (duplicate != -1)` is exactly the
//          constant-propagation shape the image shows (the sentinel-break and
//          loop-exhaust edges jump straight to the append block because they
//          carry a known -1, only the dup edge gets a real `cmp eax,-1`).
// size      527 bytes
// prototype int (__thiscall ?add_item@PullDown@@QAEHHPAD@Z)(PullDown* this, int, int8*)
// callers   12   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005F9D30 0x006453E0 0x00645470 0x00645DD0
Return Value: 0 when the item was added, else the reason it was not
Status: Complete
*/
int PullDown::add_item(int id, char *text) {
    const int new_count = ++item_count_;
    if (new_count > 64) {
        --item_count_;
        return 1;
    }
    if (text == nullptr || id == -1) {
        return 3;
    }

    // A duplicate id is refused; an unused slot is only where the item lands.
    const int duplicate = find_item(items_, id);
    if (duplicate != -1) {
        return 1;
    }

    const int slot = new_count - 1;
    items_[slot].id = id;
    if (items_[slot].text != nullptr) {
        return 0xC;
    }

    // Split the text at the '|' first, so both halves are measured from it.
    char *separator = strchr(text, '|');
    if (separator != nullptr) {
        *separator = '\0';
    }

    items_[slot].text = static_cast<char *>(mem_get(strlen(text) + 1));
    if (items_[slot].text == nullptr) {
        items_[slot].id = -1;
        --item_count_;
        return 4;
    }
    items_[slot].text[0] = '\0';
    items_[slot].mnemonic = nullptr;

    // '&x' marks the mnemonic; '&&' is an escaped ampersand; '& ' is literal.
    char *ampersand = strchr(text, '&');
    if (ampersand != nullptr && ampersand[1] != ' ') {
        items_[slot].text[0] = '\0';
        *ampersand = '\0';
        strcat(items_[slot].text, text);
        *ampersand = '&';
        if (ampersand[1] != '&') {
            items_[slot].mnemonic =
                items_[slot].text + strlen(items_[slot].text);
        }
        strcat(items_[slot].text, ampersand + 1);
    } else {
        strcat(items_[slot].text, text);
    }

    if (separator != nullptr) {
        items_[slot].right_text =
            static_cast<char *>(mem_get(strlen(separator + 1) + 1));
        if (items_[slot].right_text == nullptr) {
            items_[slot].id = -1;
            --item_count_;
            return 4;
        }
        items_[slot].right_text[0] = '\0';
        strcat(items_[slot].right_text, separator + 1);
        *separator = '|';
    }

    items_[slot].flags = 1;
    dirty_ = 1;
    compute_length(text);
    ++visible_count_;
    return 0;
}


/*
Purpose: Append a separator item to the end of the pull-down, and give it the
         requested ID. A full table answers 1, an unknown ID answers 3, and a
         slot whose text is still allocated answers 0xC.
// ORIGINAL: 0x005F8BE0 ?add_separator@PullDown@@QAEHH@Z 0x005F8BE0-0x005F8C81 BYTE_EXACT
// size      161 bytes
// prototype int (__thiscall ?add_separator@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   10   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 0 when the separator was added, else the reason it was not
Status: Complete
*/
int PullDown::add_separator(int id) {
    const int new_count = ++item_count_;
    if (new_count > 64) {
        --item_count_;
        return 1;
    }
    if (id == -1) {
        return 3;
    }
    // Reuse the first free slot if there is one; the append is only where the
    // new item lands when the table had none.
    for (int i = 0; i < 64; ++i) {
        if (items_[i].id == -1) {
            const int slot = new_count - 1;
            if (items_[slot].text != nullptr) {
                return 0xC;
            }
            items_[slot].id = id;
            items_[slot].flags = 5;
            ++visible_count_;
            dirty_ = 1;
            return 0;
        }
    }
    return 1;
}


/*
Purpose: Replace the text of the first item carrying the requested ID,
         splitting and unescaping exactly as add_item does. An empty text
         answers 3, an unknown ID answers 0xB, and an allocation failure
         answers 4.
// ORIGINAL: 0x005F8E50 ?rename_item@PullDown@@QAEHHPAD@Z 0x005F8E50-0x005F903E BYTE_EXACT
// LEVER: items_[index].field directly, not through a slot pointer - same
//          lesson as add_item above, and here it is the difference between a
//          `lea esi,[ebp+eax*4]` with 0xa18 in the displacements and a
//          `lea esi,[ebp+eax*4]` with the struct offsets there.
// TRIED: NULL guard on `id` before the scan (add_item has one); rename_item
//          has no such guard and answers 3 on the text alone - adding it
//          costs the body its first epilogue.
// size      494 bytes
// prototype int (__thiscall ?rename_item@PullDown@@QAEHHPAD@Z)(PullDown* this, int, int8*)
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005F9D30 0x00644EF2 0x006453E0 0x00645470 0x00645DD0
Return Value: 0 when the item was renamed, else the reason it was not
Status: WIP
*/
int PullDown::rename_item(int id, char *text) {
    if (text == nullptr) {
        return 3;
    }
    const int index = find_item(items_, id);
    if (index == -1) {
        return ItemNotFound;
    }

    // Split the text at the '|' first, so both halves are measured from it.
    char *separator = strchr(text, '|');
    if (separator != nullptr) {
        *separator = '\0';
    }

    if (items_[index].text != nullptr) {
        free(items_[index].text);
        items_[index].text = nullptr;
    }
    if (items_[index].right_text != nullptr) {
        free(items_[index].right_text);
        items_[index].right_text = nullptr;
    }

    items_[index].text = static_cast<char *>(mem_get(strlen(text) + 1));
    if (items_[index].text == nullptr) {
        return 4;
    }
    items_[index].text[0] = '\0';
    items_[index].mnemonic = nullptr;

    // '&x' marks the mnemonic; '&&' is an escaped ampersand; '& ' is literal.
    char *ampersand = strchr(text, '&');
    if (ampersand != nullptr && ampersand[1] != ' ') {
        items_[index].text[0] = '\0';
        *ampersand = '\0';
        strcat(items_[index].text, text);
        *ampersand = '&';
        if (ampersand[1] != '&') {
            items_[index].mnemonic =
                items_[index].text + strlen(items_[index].text);
        }
        strcat(items_[index].text, ampersand + 1);
    } else {
        strcat(items_[index].text, text);
    }

    if (separator != nullptr) {
        items_[index].right_text =
            static_cast<char *>(mem_get(strlen(separator + 1) + 1));
        if (items_[index].right_text == nullptr) {
            return 4;
        }
        items_[index].right_text[0] = '\0';
        strcat(items_[index].right_text, separator + 1);
        *separator = '|';
    }

    compute_length(text);
    dirty_ = 1;
    return 0;
}

/*
Purpose: Return the selected item index unless that item is disabled.
// ORIGINAL: 0x005F9F40 ?get_selected@PullDown@@QAEHXZ 0x005F9F40-0x005F9F5C BYTE_EXACT
// size      28 bytes
// prototype int (__thiscall ?get_selected@PullDown@@QAEHXZ)(PullDown* this)
// callers   13   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int PullDown::get_selected() {
    if (selected_ == -1) {
        return -1;
    }
    return (items_[selected_].flags & 2U) ? -1 : selected_;
}








PullDownFillCallback PullDownFieldF38Default;  // 0x009B7B58
PullDownBackfillCallback PullDownFieldF3CDefault;  // 0x009B7B5C
Font *PullDownDefaultFont;         // 0x009B7B68

// Palette indices shared by the popup policy and PullDown rendering.
// RECOVERY_OBJECT: v1 0x00696E08 ?PullDownDefaultTextColor@@3HA
int PullDownDefaultTextColor = 247;        // 0x00696E08
// RECOVERY_OBJECT: v1 0x00696E0C ?PullDownDefaultTextShadow@@3HA
int PullDownDefaultTextShadow = 255;       // 0x00696E0C
// RECOVERY_OBJECT: v1 0x00696E10 ?PullDownDefaultSelectedTextColor@@3HA
int PullDownDefaultSelectedTextColor = 7;  // 0x00696E10
// RECOVERY_OBJECT: v1 0x00696E14 ?PullDownDefaultSelectedTextShadow@@3HA
int PullDownDefaultSelectedTextShadow = 7; // 0x00696E14
// RECOVERY_OBJECT: v1 0x00696E18 ?PullDownDefaultDisabledTextColor@@3HA
int PullDownDefaultDisabledTextColor = 248;// 0x00696E18
// RECOVERY_OBJECT: v1 0x009B7B64 ?PullDownDefaultDisabledTextShadow@@3HA
int PullDownDefaultDisabledTextShadow;     // 0x009B7B64
// RECOVERY_OBJECT: v1 0x009B7B60 ?PullDownDefaultBackground@@3HA
int PullDownDefaultBackground;             // 0x009B7B60

// Shared menu geometry, initialized in the original image's .data.
// RECOVERY_OBJECT: v2 0x00696E04 0x00696E08 ?PullDownTextPadding@@3HA 0x005F9170
int PullDownTextPadding = 10;
// RECOVERY_OBJECT: v2 0x00696E1C 0x00696E20 ?PullDownBorderPadding@@3HA 0x005F9170
int PullDownBorderPadding = 2;
// These two reset words have no established event interpretation yet.
// RECOVERY_OBJECT: v2 0x009B7B70 0x009B7B74 ?PullDownState009B7B70@@3HA 0x005F9170
int PullDownState009B7B70;
// RECOVERY_OBJECT: v2 0x009B7B74 0x009B7B78 ?PullDownState009B7B74@@3HA 0x005F9170
int PullDownState009B7B74;
// Published only for the duration of the modal call at 0x005F929C.
// RECOVERY_OBJECT: v2 0x009B7B78 0x009B7B7C ?ActivePullDown@@3PAVPullDown@@A 0x005F9170
PullDown *ActivePullDown;

// ORIGINAL: 0x005F9170 ?show@PullDown@@QAEHHHP6AHXZ@Z 0x005F9170-0x005F9397
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005F9170-0x005F9397"], "reason": "Original modal menu behavior expressed through the GraphicWin base, item members and typed callback."}
int PullDown::show(int x, int y, int (*poll)()) {
    poll_ = poll;
    if (dirty_) {
        int width = width_ + 2 * (PullDownBorderPadding + PullDownTextPadding);
        int height = visible_count_ * PullDownDefaultFont->line_advance()
                   + 2 * PullDownBorderPadding;
        if (width != dib_.bmiHeader.biWidth || height != -dib_.bmiHeader.biHeight) {
            GraphicWin::resize(width, height, 0);
        }
    }
    selected_ = -1;
    if (win_parent_) {
        x = x < 0 ? 0 : x;
        y = y < 0 ? 0 : y;
        if (x + dib_.bmiHeader.biWidth > WinScreenWidth)
            x = WinScreenWidth - dib_.bmiHeader.biWidth;
        if (y - dib_.bmiHeader.biHeight > WinScreenHeight)
            y = WinScreenHeight + dib_.bmiHeader.biHeight;
    } else {
        ++x;
        ++y;
        if (x + dib_.bmiHeader.biWidth > WinScreenWidth)
            x -= dib_.bmiHeader.biWidth;
        if (y - dib_.bmiHeader.biHeight > WinScreenHeight)
            y += dib_.bmiHeader.biHeight;
        x = x < 0 ? 0 : x;
        y = y < 0 ? 0 : y;
    }
    PullDownState009B7B70 = 0;
    PullDownState009B7B74 = 0;
    Win::move(x, y);
    if (!win_parent_ && !WinPointerOwner3) {
        WinPointerOwner3 = this;
        WinPointerOwner4 = 0;
    }
    ActivePullDown = this;
    set_modal(1, poll, 0);
    ActivePullDown = 0;
    if (!win_parent_) {
        if (WinPointerOwner3 == this) {
            WinPointerOwner3 = 0;
            on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == this) WinPointerOwner4 = 0;
    }
    Win::hide();
    do_all_draws();
    if (selected_ != -1 && menu_ && !(items_[selected_].flags & 2)) {
        menu_->menu_event(selected_ < 64 ? items_[selected_].id : -1);
    }
    return selected_;
}

// ORIGINAL: 0x005F8920 ?init@PullDown@@QAEXPAUMenu@@@Z 0x005F8920-0x005F8967 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005F8920-0x005F8967", "git:7c6c86d9:src/recovered/005f8920.cpp"], "reason": "Initialize the real GraphicWin base and disable markup through its Buffer subobject; Menu is a class in product source, not the retired seed struct spelling."}
// symbol    ?init@PullDown@@QAEXPAVMenu@@@Z
void PullDown::init(Menu *menu) {
    if (PullDownDefaultFont == 0) {
        PullDownDefaultFont = FontDefault;
    }
    menu_ = menu;
    GraphicWin::init(0, 0, 0, 0, 0, 0, 0, 0, 0);
    set_markup_enabled(0);
}

/*
Purpose: Destroy a PullDown by releasing every item's text pair, resetting
         the trailing fields from their global defaults, and delegating to
         the GraphicWin destructor.
// ORIGINAL: 0x005F88A0 ??1PullDown@@QAE@XZ 0x005F88A0-0x005F891B
// symbol    ??1PullDown@@UAE@XZ
// size      123 bytes
// prototype void (__thiscall ??1PullDown@@QAE@XZ)(PullDown* this)
// callers   27   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00644EF2
The compiler owns table staging and base teardown. Earlier destroy()-helper
experiments are retained in Git; they do not describe this real destructor.
*/
PullDown::~PullDown() {
    // Sixty-four items with two owned strings each, released directly
    // through items_'s own named fields.
    for (size_t index = 0; index < 64; ++index) {
        if (items_[index].text != nullptr) {
            free(items_[index].text);
            items_[index].text = nullptr;
        }
        if (items_[index].right_text != nullptr) {
            free(items_[index].right_text);
            items_[index].right_text = nullptr;
        }
    }

    dirty_ = 1;
    field_F38_ = PullDownFieldF38Default;
    field_F3C_ = PullDownFieldF3CDefault;
    // GraphicWin is destroyed once by C++ after the owned strings.
}


/*
Purpose: Legacy stub; the original body returns 8 without reading its
         arguments.
// ORIGINAL: 0x005F8C90 ?UNK2@PullDown@@QAEHH@Z 0x005F8C90-0x005F8C98 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?UNK2@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int PullDown::UNK2(int) {
    return 8;
}


/*
Purpose: Legacy stub; the original body returns 8 without reading its
         arguments.
// ORIGINAL: 0x005F8CA0 ?UNK3@PullDown@@QAEHHHH@Z 0x005F8CA0-0x005F8CA8 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?UNK3@PullDown@@QAEHHHH@Z)(PullDown* this, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int PullDown::UNK3(int, int, int) {
    return 8;
}


/*
Purpose: Legacy stub; the original body returns 1 without reading its
         arguments.
// ORIGINAL: 0x005FA750 ?UNK5@PullDown@@QAEHXZ 0x005FA750-0x005FA756 BYTE_EXACT
// size      6 bytes
// prototype int (__thiscall ?UNK5@PullDown@@QAEHXZ)(PullDown* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int PullDown::UNK5() {
    return 1;
}


/*
Purpose: Legacy stub; the original body returns 1 without reading its
         arguments.
// ORIGINAL: 0x005FA760 ?UNK6@PullDown@@QAEHXZ 0x005FA760-0x005FA766 BYTE_EXACT
// size      6 bytes
// prototype int (__thiscall ?UNK6@PullDown@@QAEHXZ)(PullDown* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int PullDown::UNK6() {
    return 1;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005FA740 ?on_mouse_leave@PullDown@@QAEXHH@Z 0x005FA740-0x005FA743 BYTE_EXACT
// symbol    ?on_mouse_leave@PullDown@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_leave@PullDown@@QAEXHH@Z)(PullDown* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void PullDown::on_mouse_leave(int, int) {
}


/*
Purpose: Find a menu item by its id and return its index, scanning up to the
         64 item slots. Stops at the first slot whose id is the -1 sentinel,
         and returns -1 when the id is not found or that sentinel is reached.
// ORIGINAL: 0x005F9D00 ?id_to_index@PullDown@@QAEHH@Z 0x005F9D00-0x005F9D28 BYTE_EXACT
// size      40 bytes
// prototype int (__thiscall ?id_to_index@PullDown@@QAEHH@Z)(PullDown* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the item's index, or -1 when absent
Status: Complete
*/
int PullDown::id_to_index(int id) {
    for (int index = 0; index < 64; ++index) {
        const int slot_id = items_[index].id;
        if (slot_id == -1) {
            return -1;
        }
        if (slot_id == id) {
            return index;
        }
    }
    return -1;
}


/*
Purpose: Resolve width_ to the widest item string, measuring the mnemonically
         stripped text plus twice the two-space separator plus the right half.
         A null text re-measures the whole table; a named text measures only
         that one.
// ORIGINAL: 0x005F9D30 ?compute_length@PullDown@@QAEHPAD@Z 0x005F9D30-0x005F9F34
// size      516 bytes
// prototype int (__thiscall ?compute_length@PullDown@@QAEHPAD@Z)(PullDown* this, int8*)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00619280 0x006192F0 0x00645DD0
// WHAT IT IS: the lazy `PullDownDefaultFont` slot (0x009B7B68, resolved once
//          from FontDefault) plus the width of the widest item string. Each
//          arm is the same `width_ = max(width_, <measurement>)` with the max
//          written OUT as an if that re-evaluates its second operand - 16
//          calls to Font::width for 8 measurements, because the macro
//          re-evaluates exactly as Font::width(LPSTR, size_t)'s own documented
//          min does. The four `push imm32` string arguments are four distinct
//          instances of the same "  " literal at 0x696E20/24/28/2C, one per
//          macro expansion in this function.
// MISMATCH 7 of 162 (compiled 144) - RECEIVER-SPILL WALL, recognised by the
//          brief's fingerprint and not reached in four measured spellings.
//          The image opens `push ecx / push ebx / push ebp / mov ebp,ecx /
//          push esi / push edi / test / mov [esp+0x10],ebp`, so `this` lives
//          in a reserved stack slot and ebp is free as the accumulator in the
//          whole-table arm; it reloads it at 0x005F9EE6. This tree keeps
//          `this` in ebp for the whole body and never spills, so it is 18
//          instructions short and the accumulator lands in ebx instead.
// TRIED: `PullDownItem *item = &items_[i]` in the loop -> `items_[i].field`
//          (the add_item lesson), 5/162 -> 9/162.
// TRIED: hoisting `width_` into `const int current` before the width() calls
//          in both arms, matching the image's `mov edi,[ebp+0xf24]` before the
//          call rather than a folded `cmp [ebp+0xf24],eax` - moved the loop
//          but not the prologue.
// TRIED: reordering the summand to `2 * width("  ") + width(text) +
//          width(right)` to match the image's evaluation order (pipe, text,
//          right; ours was pipe, right, text) - no movement at all.
// TRIED: renaming via `PullDown::rename_item`'s shape, which DOES reproduce
//          the identical `push ecx` + `mov [esp+0x10],ebp` spill and is
//          BYTE_EXACT - so the spill is reachable from source, this body just
//          needs one more live register than this spelling supplies. Left
//          here rather than hand-spelled; see the note on rename_item.
Status: WIP
*/
int PullDown::compute_length(char *text) {
    if (PullDownDefaultFont == nullptr) {
        PullDownDefaultFont = FontDefault;
    }

    if (text != nullptr) {
        char *separator = strchr(text, '|');
        if (separator == nullptr) {
            const int current = width_;
            int measured = PullDownDefaultFont->width(text);
            if (current <= measured) {
                measured = PullDownDefaultFont->width(text);
            } else {
                measured = current;
            }
            width_ = measured;
            return width_;
        }

        const int previous = width_;
        int left_width = PullDownDefaultFont->width(text, separator - text);
        int separator_width = PullDownDefaultFont->width("  ");
        int right_width = PullDownDefaultFont->width(separator + 1);
        int total = static_cast<int32_t>(static_cast<uint32_t>(left_width)
                    + 2U * static_cast<uint32_t>(separator_width)
                    + static_cast<uint32_t>(right_width));
        if (previous > total) {
            width_ = previous;
            return width_;
        }
        separator_width = PullDownDefaultFont->width("  ");
        left_width = PullDownDefaultFont->width(text, separator - text);
        right_width = PullDownDefaultFont->width(separator + 1);
        width_ = static_cast<int32_t>(static_cast<uint32_t>(left_width)
                 + 2U * static_cast<uint32_t>(separator_width)
                 + static_cast<uint32_t>(right_width));
        // 0x005F9DF9 leaves the final right-side measurement in EAX.
        // This is deliberately not the aggregate stored in width_.
        return right_width;
    } else {
        width_ = 0;
        for (int i = 0; i < 64; ++i) {
            char *const item_text = items_[i].text;
            if (item_text == nullptr) {
                continue;
            }
            char *const first_right_text = items_[i].right_text;
            const int current = width_;
            if (first_right_text == nullptr) {
                int measured = PullDownDefaultFont->width(item_text);
                if (current <= measured) {
                    measured = PullDownDefaultFont->width(items_[i].text);
                } else {
                    measured = current;
                }
                width_ = measured;
            } else {
                int separator_width = PullDownDefaultFont->width("  ");
                int left_width = PullDownDefaultFont->width(items_[i].text);
                int right_width = PullDownDefaultFont->width(first_right_text);
                int total = static_cast<int32_t>(2U * static_cast<uint32_t>(separator_width)
                            + static_cast<uint32_t>(left_width)
                            + static_cast<uint32_t>(right_width));
                if (current > total) {
                    total = current;
                } else {
                    separator_width = PullDownDefaultFont->width("  ");
                    right_width = PullDownDefaultFont->width(items_[i].right_text);
                    left_width = PullDownDefaultFont->width(items_[i].text);
                    total = static_cast<int32_t>(2U * static_cast<uint32_t>(separator_width)
                            + static_cast<uint32_t>(left_width)
                            + static_cast<uint32_t>(right_width));
                }
                width_ = total;
            }
        }
    }

    // 0x005F9F21 exhausts the 64-entry loop counter in EAX before RET.
    // Encode the emitted result instead of falling off an int function.
    return 0;
}


/*
// ORIGINAL: 0x005F93A0 ?hide@PullDown@@QAEXXZ 0x005F93A0-0x005F93B9 BYTE_EXACT
// symbol    ?hide@PullDown@@UAEXXZ
// size      25 bytes
// prototype void (__thiscall ?hide@PullDown@@QAEXXZ)(PullDown* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EDCD0 0x005FCBB0
// indirect  0x005F93A5
Status: Complete
*/
void PullDown::hide() {
    release_modal();
    reinterpret_cast<Win *>(this)->Win::hide();
    do_all_draws();
}


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x005F8800 ??0PullDown@@QAE@XZ 0x005F8800-0x005F8893
// size      147 bytes
// prototype void (__thiscall ??0PullDown@@QAE@XZ)(PullDown* this)
// callers   26   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4CF0
*/

// The one base-constructor call in the image belongs to implicit C++ base
// initialization. Placement-constructing GraphicWin here ran it twice.
PullDown::PullDown() {
    for (size_t index = 0; index < 64; ++index) {
        items_[index].text = nullptr;
        items_[index].right_text = nullptr;
        items_[index].id = -1;
        items_[index].flags = 0;
        items_[index].mnemonic = nullptr;
    }

    // STORE ORDER, NOT DECLARATION ORDER: the image writes menu_ (0xF30)
    // before visible_count_ (0xF2C), and field_A14_ (0xA14, the class's
    // FIRST declared field) dead last, after dirty_.
    field_F18_ = 0;
    poll_ = 0;
    item_count_ = 0;
    width_ = 0;
    selected_ = -1;
    menu_ = nullptr;
    visible_count_ = 0;
    dirty_ = 1;
    field_A14_ = 0;
    field_F38_ = PullDownFieldF38Default;
    field_F3C_ = PullDownFieldF3CDefault;
}

/*
Purpose: Step the receiver back to the subobject ??_GPullDown@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x005FA790 ??_GPullDown@@WEEE@AEPAXI@Z 0x005FA790-0x005FA79B BYTE_EXACT
// symbol    ??_EPullDown@@WEEE@AEPAXI@Z
// CORRECTED from ??3PullDown@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005FA770` into
//   ??_GPullDown@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// ORIGINAL: 0x005F9460 ?on_right_down@PullDown@@QAEXHH@Z 0x005F9460-0x005F9472 BYTE_EXACT
// symbol    ?on_right_down@PullDown@@UAEXHH@Z
// size      18 bytes
// prototype void (__thiscall ?on_right_down@PullDown@@QAEXHH@Z)(PullDown* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F946C
// 0x005F9460  ?on_right_down@PullDown@@QAEXHH@Z  ->  ?on_right_down@PullDown@@QAEXHH@Z
//
// Purpose: Pass a right-button press straight through to Win's virtual slot
//          20 - the image's `call dword ptr [eax + 0x50]` - where the
//          pull-down's owner decides what a right click means.


void PullDown::on_right_down(int x, int y) {
    on_left_down(x, y);
}

// ORIGINAL: 0x005F9570 ?on_right_up@PullDown@@QAEXHH@Z 0x005F9570-0x005F9582 BYTE_EXACT
// symbol    ?on_right_up@PullDown@@UAEXHH@Z
// size      18 bytes
// prototype void (__thiscall ?on_right_up@PullDown@@QAEXHH@Z)(PullDown* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F957C
// 0x005F9570  ?on_right_up@PullDown@@QAEXHH@Z  ->  ?on_right_up@PullDown@@QAEXHH@Z
//
// Purpose: Pass a right-button release straight through to Win's virtual slot
//          21 - the image's `call dword ptr [eax + 0x54]` - the counterpart
//          of on_right_down one slot up.


void PullDown::on_right_up(int x, int y) {
    on_left_up(x, y);
}

// ORIGINAL: 0x005F93C0 ?on_left_down@PullDown@@QAEXHH@Z 0x005F93C0-0x005F945B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5F93C0..5F945B; retired seed git:117282ee:src/unrecovered/005f93c0.cpp records the same identity and spans.", "Pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: PullDown constructor 0x5F8800 stores vftable 0x66FF40; its slot-20 cell 0x66FF90 contains 0x5F93C0 and slot-21 cell 0x66FF94 contains 0x5F9480.", "Pinned Menu constructor 0x5FAC94 stores vftable 0x6700B0; Menu slot-20 cell 0x670100 contains 0x5FBC20, matching PullDown's outside-click virtual call through +0x50.", "The typed owner declarations are Win::on_left_down/on_left_up, GraphicWin::buffer_working_rect (returning Buffer rect2_ at +0x474), Win::outer_rect (RECT at +0x13C), and Menu::on_left_down; PullDown already owns item_count_, selected_, menu_, PullDownDefaultFont and PullDownItem[64].", "Pinned 0x5F93C0..0x5F945B reads rect2_ +0x474; outside path calls PullDown slot 2, flush_input, do_all_tasks(this), then reads menu_ +0xF30, clears selected_ +0xF28, and calls Menu slot 20 with outer-RECT deltas plus click coordinates."], "reason": "Move the original outside-click handler into PullDown's real Win slot-20 override; preserve its hide/flush/tasks/menu-coordinate sequence."}
// symbol    ?on_left_down@PullDown@@UAEXHH@Z
void PullDown::on_left_down(int x, int y) {
    const RECT rc = *buffer_working_rect();
    const POINT pt = {x, y};
    if (PtInRect(&rc, pt) == 0) {
        hide();
        flush_input();
        do_all_tasks(static_cast<Win *>(this));
        Menu *const menu = menu_;
        selected_ = -1;
        if (menu != nullptr) {
            const RECT &self_rect = outer_rect();
            const RECT &menu_rect = menu->outer_rect();
            const int translated_y = (self_rect.top - menu_rect.top) + y;
            const int translated_x = (self_rect.left - menu_rect.left) + x;
            menu->on_left_down(translated_x, translated_y);
        }
    }
}

// ORIGINAL: 0x005F9480 ?on_left_up@PullDown@@QAEXHH@Z 0x005F9480-0x005F9563 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5F9480..5F9563; retired seed git:117282ee:src/unrecovered/005f9480.cpp retains original identity and spans.", "Pinned PullDown constructor5F8800 installs66FF40: slot21 points5F9480, slot2 hide5F93A0, slot62 GraphicWin redraw5D5A70.", "Current PullDown owns items_ at+A18 with0x14-byte rows; PullDownItem flags at+C gives the original+A24+i*14 test address. Constructor/add/redraw and native emitted member accesses agree; this is not a text-pointer tag test.", "Pinned release uses flag masks1/4, signed line-height division, count+F20 and selected+F28; preserve zero/negative counts, empty redraw and selected hide order without added guards."], "reason": "The original release selector belongs to PullDown Win slot 21; express its existing item flags and callbacks through the real owner, preserving the original count/error paths."}
// symbol    ?on_left_up@PullDown@@UAEXHH@Z
void PullDown::on_left_up(int x, int y) {
    const RECT rect = *buffer_working_rect();
    const POINT pt = {x, y};
    if (PtInRect(&rect, pt) != 0) {
        Font *const font = PullDownDefaultFont;
        const int line_height = font->line_advance();
        const int count = item_count_;
        selected_ = (y - PullDownBorderPadding) / line_height;
        int index = 0;
        if (count > 0) {
            int enabled = -1;
            do {
                if ((items_[index].flags & 1) != 0) {
                    enabled++;
                }
                if (enabled == selected_) {
                    break;
                }
                index++;
            } while (index < count);
        }
        if (index == count) {
            selected_ = -1;
        } else {
            selected_ = index;
            if ((items_[index].flags & 4) != 0) {
                selected_ = -1;
            }
        }
        if (selected_ == -1) {
            redraw();
            return;
        }
        hide();
    }
}

// ORIGINAL: 0x005F9100 ?UNK4@PullDown@@QAEXXZ 0x005F9100-0x005F9165 BYTE_EXACT
// size      101 bytes
// prototype void (__thiscall ?UNK4@PullDown@@QAEXXZ)(PullDown* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// 0x005F9100  ?UNK4@PullDown@@QAEXXZ  ->  ?UNK4@PullDown@@QAEXXZ
//
// Purpose: release every item's text pair and clear the item table, leaving
//          the pull-down with no items and the redraw latch set.


void PullDown::UNK4() {
    // THE IMAGE BASES ITS WALK POINTER ON THE ITEM'S SECOND MEMBER, not on
    // the item: `lea esi,[ebp+0xa1c]` is &items_[0].right_text, and every
    // access in the loop is a displacement off that pointer - [-4] text,
    // [0] right_text, [+4] id, [+8] flags. Basing a PullDownItem* on the
    // item instead shifts all four displacements by four bytes (same
    // mnemonics, wrong bytes), so the walk is spelled over the text slot.
    // cursor[1] and cursor[2] are the id and flags words that follow the two
    // owned strings in PullDownItem.
    char **cursor = &items_->right_text;
    int count = 0x40;
    do {
        if (cursor[-1] != nullptr) {
            free(cursor[-1]);
            cursor[-1] = nullptr;
        }
        if (cursor[0] != nullptr) {
            free(cursor[0]);
            cursor[0] = nullptr;
        }
        cursor[1] = reinterpret_cast<char *>(-1);
        cursor[2] = nullptr;
        cursor += sizeof(PullDownItem) / sizeof(char *);
    } while (--count != 0);
    field_F18_ = 0;
    item_count_ = 0;
    visible_count_ = 0;
    dirty_ = 1;
}

// ORIGINAL: 0x005F98B0 ?on_redraw@PullDown@@QAEXXZ 0x005F98B0-0x005F9CFF
// RECOVERY_BINDING: v1 {"evidence": ["task:pulldown-flatbutton-renderer-and-sprite-ownership", "historical seed:src/recovered/units/005f98b0.cpp", "Pinned original005F98B0; full native instructions, current class and producer/consumer closure."], "reason": "Recover the complete redraw in its genuine virtual C++ owner using real source-owned members, callbacks, sprites and Buffer operations; pointer configuration is borrowed, not owned."}
// symbol    ?on_redraw@PullDown@@UAEXXZ
void PullDown::on_redraw() {
  Buffer *const buffer = static_cast<Buffer *>(this);

  if (PullDownDefaultFont == 0) {
    PullDownDefaultFont = FontDefault;
  }

  if (buffer->set_font(PullDownDefaultFont, 0, 0, 0) != 0) {
    return;
  }

  // The image snapshots its working rectangle before calling either custom
  // backfill path. The callback can paint through this PullDown, but the item
  // geometry below continues from this original rectangle snapshot.
  const RECT working = buffer->clip_rect();
  int top = working.top;
  int bottom = working.bottom;
  int left = working.left;
  int right = working.right;

  field_F38_ = PullDownFieldF38Default;
  field_F3C_ = PullDownFieldF3CDefault;

  if (field_F3C_ == 0) {
    fill(PullDownDefaultBackground);

    int border_index = 0;
    if (PullDownBorderPadding > 0) {
      RECT border = {left, top, right, bottom};
      do {
        buffer->box(&border, PullDownDefaultSelectedTextShadow,
                    PullDownDefaultDisabledTextColor);
        ++border.left;
        --border.right;
        ++border.top;
        --border.bottom;
        ++border_index;
      } while (border_index < PullDownBorderPadding);
      left = border.left;
      top = border.top;
      right = border.right;
      bottom = border.bottom;
    }
    left -= PullDownBorderPadding;
    right += PullDownBorderPadding;
    top -= PullDownBorderPadding;
    bottom += PullDownBorderPadding;
  } else {
    field_F3C_(this);
  }

  const int available_width = right - left;
  int row_bottom = PullDownBorderPadding + buffer->text_line_height();
  int row_top = PullDownBorderPadding;
  int row_left = PullDownTextPadding + PullDownBorderPadding;
  row_bottom = PullDownBorderPadding + buffer->text_line_height();
  int row_right = available_width - row_left;

  if (item_count_ > 0) {
    for (int index = 0; index < item_count_; ++index) {
      PullDownItem &item = items_[index];
      const uint32_t flags = item.flags;
      char *const text = item.text;
      char *const right_text = item.right_text;
      char *const mnemonic = item.mnemonic;

      if ((flags & 1u) != 0) {
        if ((flags & 8u) != 0) {
          Sprite *const checked_icon = CheckBoxDefault2;
          const int icon_y =
              ((row_bottom - checked_icon->height()) - row_top) / 2 + row_top;
          checked_icon->draw(buffer, checked_icon->transparent_index(),
                             row_left, icon_y, 1, 1);
          row_left += checked_icon->width();
        }

        int text_color = PullDownDefaultDisabledTextColor;
        if ((flags & 4u) == 0) {
          int text_shadow;
          if (index == selected_) {
            if (field_F38_ == 0) {
              RECT selected_row = {
                  row_left - PullDownTextPadding, row_top,
                  row_right + PullDownTextPadding, row_bottom};
              fill(&selected_row, PullDownDefaultBackground);
              row_left = selected_row.left + PullDownTextPadding;
              row_right = selected_row.right - PullDownTextPadding;
            } else {
              RECT selected_row = {row_left, row_top, row_right, row_bottom};
              field_F38_(this, 1, &selected_row);
              row_left = selected_row.left;
              row_top = selected_row.top;
              row_right = selected_row.right;
              row_bottom = selected_row.bottom;
            }
            text_color = PullDownDefaultTextColor;
            text_shadow = PullDownDefaultTextShadow;
          } else {
            text_color = PullDownDefaultSelectedTextColor;
            text_shadow = PullDownDefaultDisabledTextShadow;
            if (field_F38_ != 0) {
              RECT row = {row_left, row_top, row_right, row_bottom};
              field_F38_(this, 0, &row);
              row_left = row.left;
              row_top = row.top;
              row_right = row.right;
              row_bottom = row.bottom;
              text_color = PullDownDefaultSelectedTextColor;
              text_shadow = PullDownDefaultDisabledTextShadow;
            }
          }

          if ((flags & 2u) == 0) {
            text_color = text_shadow;
          }
          buffer->set_text_color(text_color, -1, 1, 1);

          if (text != 0) {
            RECT text_area = {row_left, row_top, row_right, row_bottom};
            buffer->write_l(text, &text_area, static_cast<int>(strlen(text)));
          }

          if (mnemonic != 0) {
            const int underline_color = buffer->foreground_color();
            const int font_leading = PullDownDefaultFont->internal_leading();
            const int letter_width = buffer->text_width(mnemonic, 1);
            const int line_height = buffer->text_line_height();
            const int underline_y =
                row_bottom - ((row_bottom - row_top - line_height) / 2)
                - font_leading;
            const int prefix_width = buffer->text_width(
                text, static_cast<int>(mnemonic - text));
            buffer->hline(row_left + prefix_width,
                          row_left + prefix_width + letter_width,
                          underline_y, underline_color);
          }

          if (right_text != 0) {
            RECT text_area = {row_left, row_top, row_right, row_bottom};
            buffer->write_right_l(right_text, &text_area,
                                  static_cast<int>(strlen(right_text)));
          }
        } else {
          const int first_y = buffer->text_line_height() / 2 + row_top;
          const int separator_width =
              buffer->pixel_width() - 2 * PullDownBorderPadding;
          buffer->hline(PullDownBorderPadding,
                        PullDownBorderPadding + separator_width, first_y,
                        PullDownDefaultDisabledTextColor);
          const int second_y = buffer->text_line_height() / 2 + 1 + row_top;
          buffer->hline(PullDownBorderPadding,
                        PullDownBorderPadding + separator_width, second_y,
                        PullDownDefaultSelectedTextShadow);
        }

        if ((flags & 8u) != 0) {
          row_left -= CheckBoxDefault2->width();
        }
        const int line_height = buffer->text_line_height();
        row_top += line_height;
        row_bottom += line_height;
      }

    }
  }
}
