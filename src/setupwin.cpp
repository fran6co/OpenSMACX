/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
// ORIGINAL: 0x004AE760 ??_GSetupWin@@UAEPAXI@Z 0x004AE760-0x004AE781 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004AE760-0x004AE781", "Pinned class constructor ??0SetupWin@@QAE@XZ at 0x004AD880 stores original class/view table VA 0x66d8e8 at 0x004AD957 (mov dword ptr [esi], 0x66d8e8); the current genuine owner vftable ??_7SetupWin@@6BWin@@@ selects generated ??_GSetupWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_ESetupWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GSetupWin@@UAEPAXI@Z at 0x004AE760 calls the source-mapped ??1SetupWin@@QAE@XZ at 0x004AE790, then tests byte ptr [ebp + 8], 1; je 0x4ae77a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/setupwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original SetupWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GSetupWin@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "interface_fonts.h"
#include "setupwin.h"
#include "filewin.h"
#include "game.h"
#include "popup.h"
#include "dialog.h"
#include "wave.h"
#include "palette.h"
#include "text.h"
#include "general.h"
#include "stringstruct.h"
#include <cstring>

// 0x008A6268 is the sole original reference to this process-wide filename
// scratch space (SetupWin::do_menu at 0x004ADB4B). The right-side menu may
// append a filename suffix. Its original extent is not established; reserve
// ample product storage without asserting an exact original data span.
// RECOVERY_OBJECT: v1 0x008A6268 ?SetupMenuFileScratch@@3PADA
char SetupMenuFileScratch[1024];

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004AE340 ?on_left_click@SetupWin@@QAEXHH@Z 0x004AE340-0x004AE380
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004ae340.cpp", "image:terranx_original.exe:0x004AE340", "Native4AE349/4AE368 preserves original y in EDI across Spot output mutation; feedback receives incoming coordinate.", "recovery/tasks/interface-contract-report-closure.json; bounded UI-five scope preserves original baseline"], "reason": "Recover the original in its genuine UI owner using actual members, globals and base/virtual calls. Retain seed evidence and historical obligations; this binding does not grant a current matching or runtime claim."}
// symbol    ?on_left_click@SetupWin@@UAEXHH@Z
void SetupWin::on_left_click(int x, int y) {
    // Spot may overwrite its output; the original saves the incoming y in EDI
    // and passes that saved coordinate to the virtual feedback callback.
    const int original_y = y;
    if (spot_.check(x, y, &y, 0) != -1) {
        on_mouse_move(x, original_y);
        release_modal();
    }
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x004ADAD0 ?UNK3@SetupWin@@QAEHH@Z 0x004ADAD0-0x004ADAD5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK3@SetupWin@@QAEHH@Z)(SetupWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int SetupWin::UNK3(int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x004ADAE0 ?UNK4@SetupWin@@QAEHH@Z 0x004ADAE0-0x004ADAE5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK4@SetupWin@@QAEHH@Z)(SetupWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int SetupWin::UNK4(int) {
    return 0;
}


/*
Purpose: Scale the argument by the field at 0xA14 and divide by 1024.

             mov eax,[ecx+0xA14] / imul eax,[ebp+8] / cdq
             and edx,0x3FF / add eax,edx / sar eax,0xA

         `cdq / and / add / sar` is MSVC's signed divide by a power of two: it
         biases a negative dividend by 2^n - 1 before the arithmetic shift so
         the quotient truncates toward zero, which is what C++ `/ 1024` means.
         Verified over 200,000 random 32-bit dividends plus the boundaries -
         the idiom and `/ 1024` agree on every one.

         The multiply is a 32-bit `imul`, so an overflowing product wraps and
         the division sees the wrapped value; `int` arithmetic reproduces that.

         RECOVERY_HISTORY: The former offset-based version said SetupWin
         RECOVERY_HISTORY: models no fields - it derives from GraphicWin,
         RECOVERY_HISTORY: which ends at 0xA14 - so this is its first own field,
         RECOVERY_HISTORY: reached through a documented raw offset as
         RECOVERY_HISTORY: BasePop::UNK3 does.

         The owner header declares field_A14_ and field_A18_ at these offsets.
         This body reads field_A14_ directly; its semantic name remains unknown.
// ORIGINAL: 0x004ADA80 ?UNK1@SetupWin@@QAEHH@Z 0x004ADA80-0x004ADA9D BYTE_EXACT
// size      29 bytes
// prototype int (__thiscall ?UNK1@SetupWin@@QAEHH@Z)(SetupWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: (field_A14 * a1) / 1024, truncated toward zero
Status: Complete
*/
int SetupWin::UNK1(int pos) {
    const int32_t scale = static_cast<int32_t>(field_A14_);
    return static_cast<int32_t>(static_cast<uint32_t>(scale)
                                * static_cast<uint32_t>(pos)) / 1024;
}


/*
Purpose: As UNK1, over the field at 0xA18 and dividing by 768.

             mov ecx,[ecx+0xA18] / mov eax,0x2AAAAAAB / imul ecx,[ebp+8]
             imul ecx / mov eax,edx / sar eax,7 / mov ecx,eax
             shr ecx,0x1F / add eax,ecx

         The magic-number form: 0x2AAAAAAB taken as the high half of a signed
         multiply, shifted right by 7 and corrected by the sign bit, is
         division by 768. Verified the same way as UNK1 - 200,000 random
         dividends and the boundaries, agreeing with `/ 768` on every one.

         1024 and 768 beside each other read as a resolution, which is a
         plausible account of what these two scale and is NOT what the recovery
         rests on; the arithmetic is.
// ORIGINAL: 0x004ADAA0 ?UNK2@SetupWin@@QAEHH@Z 0x004ADAA0-0x004ADAC4 BYTE_EXACT
// size      36 bytes
// prototype int (__thiscall ?UNK2@SetupWin@@QAEHH@Z)(SetupWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: (field_A18 * a1) / 768, truncated toward zero
Status: Complete
*/
int SetupWin::UNK2(int pos) {
    const int32_t scale = static_cast<int32_t>(field_A18_);
    return static_cast<int32_t>(static_cast<uint32_t>(scale)
                                * static_cast<uint32_t>(pos)) / 768;
}


/*
// ORIGINAL: 0x004ADAF0 ?do_menu@SetupWin@@QAEHPADHH@Z 0x004ADAF0-0x004ADB11 BYTE_EXACT
// size      33 bytes
// prototype int (__thiscall ?do_menu@SetupWin@@QAEHPADHH@Z)(SetupWin* this, int8*, int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004ADB70
Status: Complete
*/
int SetupWin::do_menu(char* a1, int a2, int a3) {
    switch (a2) {
        case 1:
            return do_menu_rightside(a1, a3);
        default:
            return -1;
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004ADB20 ?do_menu@SetupWin@@QAEHPAUPopup@@HH@Z 0x004ADB20-0x004ADB66 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/004adb20.cpp; image:0x004ADB20-0x004ADB66; source:src/setupwin.h:SetupWin"], "reason": "Original method stores the incoming Popup at receiver+0x1014, dispatches menu kind 1 through do_menu_rightside with the original process-wide filename scratch address, then clears the non-owning popup member on both paths."}
// symbol    ?do_menu@SetupWin@@QAEHPAVPopup@@HH@Z
int SetupWin::do_menu(Popup *popup, int menu_kind, int flags) {
    menu_popup_ = popup;
    switch (menu_kind) {
    case 1: {
        const int result = do_menu_rightside(SetupMenuFileScratch, flags);
        menu_popup_ = nullptr;
        return result;
    }
    default:
        menu_popup_ = nullptr;
        return -1;
    }
}

// Complete owner recovery for SetupWin::do_menu_rightside(char *, int).
// Uses the actual source-owned SetupWin/GraphicWin/Win,
// Buffer, Flic, Wave, Palette, Font, Spot, StringList and Popup/Dialog APIs.
// Current byte restoration remains an obligation, not a coverage prerequisite.
// Seed path below was retired; Git retains its historical provenance.
// ORIGINAL: 0x004ADB70 ?do_menu_rightside@SetupWin@@QAEHPADH@Z 0x004ADB70-0x004AE336;0x0065960C-0x00659697
// RECOVERY_BINDING: v1 {"evidence": ["pinned 4ADB70..4AE336 and EH65960C..659697", "historical seed src/unrecovered/004adb70.cpp", "full owner/pending native overlay and observed current QAEHPADH symbol", "actual-source host controls with original text-get ordering, signed DIB geometry, Popup selection and explicit/automatic cleanup"], "reason": "Complete SetupWin right-side menu through genuine inherited members and Popup/Dialog APIs, preserving native branch, error-path and cleanup order; no original-address forwarding."}
// kind      game
// symbol    ?do_menu_rightside@SetupWin@@QAEHPADH@Z
// TRIED: SEH-framed function with 4 real local objects (Buffer, Font, Wave, Flic) - reproduced via plain RAII locals (`Buffer buf1; Font font; Wave wave; Flic flic;` with `Palette pal;` scoped to end right before the final destructor cascade, matching the original's separate ~Palette() call ahead of the buf1/font/wave/flic teardown) and plain `return`/goto for the early-exit paths, letting the compiler emit its own SEH prologue/epilogue and destructor calls rather than hand-writing the fs:[0] chain or the mid-function state-index byte stores. Diverges at #2 (framed EH prologue ordering) - did not chase further given the size; body is a full instruction-level transcription of the menu/flic drawing logic (token-line parsing loop, two flic decode/blit/pace loops via a shared static helper, Spot::add layout loop, draw_item loop) with call arguments cross-checked against raw disassembly.
int SetupWin::do_menu_rightside(char *filename, int flags) {
    Buffer scratch;
    Font font;
    Wave wave;
    Flic flic;

    selected_row_ = 0;
    row_count_ = 0;
    field_A14_ = 0;
    field_A18_ = 0;
    buffer_.close();
    spot_.shutdown();
    field_FE0_.remove_all();
    field_FE0_.current_position_ = 0;
    field_A14_ = WinScreenWidth;
    field_A18_ = WinScreenHeight;

    if (opening_window_snapshot_ == nullptr) {
        flic.close();
        return -1;
    }
    if (filename == nullptr && menu_popup_ == nullptr) {
        flic.close();
        return -1;
    }
    if (menu_popup_ == nullptr && X_text_open(filename, "script.txt") != 0) {
        flic.close();
        return -1;
    }

    int result = 0;
    {
        Palette palette;
        char token[512];
        bool has_token = false;
        RECT title_rect;

        field_FE0_.remove_all();
        field_FE0_.current_position_ = 0;
        field_FE0_.field_20_ = 0;

        font.init(const_cast<char *>(DefaultFontFace), 0x12, 1);
        wave.load("fx\\menu out.wav");
        selected_row_ = static_cast<uint32_t>(flags);
        token[0] = '\0';

        if (menu_popup_ == nullptr) {
            bool seen_blank = false;
            row_count_ = 0;
            for (;;) {
                char *line = text_get();
                // Preserve the source transcription's ordering: the original
                // dereferences line to test '#' before checking the null exit.
                // Whether text_get can return null here is a native-runtime
                // question, not an invitation to silently reorder the branch.
                if (line[0] == '#') {
                    if (_strnicmp(line + 1, "caption", 7) == 0) {
                        purge_spaces(line + 8);
                        purge_leading(line + 8);
                        token[0] = '\0';
                        strcat(token, line + 8);
                    }
                    continue;
                }
                if (line == nullptr) {
                    break;
                }
                if (line[0] == '\0') {
                    if (seen_blank) {
                        break;
                    }
                    seen_blank = true;
                } else {
                    const int position = static_cast<int>(row_count_++);
                    field_FE0_.pending_text_ = line;
                    field_FE0_.field_20_ = 0;
                    field_FE0_.field_24_ = 0;
                    field_FE0_.add(position);
                    seen_blank = true;
                }
            }
            item_texts_ = &field_FE0_;
            text_close();
        } else {
            Dialog &dialog = static_cast<Dialog &>(menu_popup_->dialogs());
            item_texts_ = &dialog.item_list_;
            row_count_ = static_cast<uint32_t>(dialog.item_list_.entry_count_);
            if (menu_popup_->value() >= 0) {
                selected_row_ = static_cast<uint32_t>(menu_popup_->value());
            }
            const char *caption = menu_popup_->caption_text();
            if (caption != nullptr) {
                strcat(filename, caption);
            }
        }

        has_token = token[0] != '\0';
        spot_.init(static_cast<int>(row_count_) + 2);
        buffer2_.init(0x162, 0x4B, 0, nullptr);
        flic.init(&buffer2_, const_cast<char *>("openitem.flc"), 0, 0, 0);

        // Native arithmetic carries DIB::biHeight (negative for this
        // top-down surface) as-is. Do not normalize it to pixel_height().
        const int line_stride = static_cast<int>(buffer2_.dib_.bmiHeader.biHeight);
        const int menu_width = static_cast<int>(buffer2_.dib_.bmiHeader.biWidth);
        const int menu_rows = static_cast<int>(row_count_);
        int menu_height = -(line_stride * menu_rows);
        if (has_token) {
            menu_height += 0x14 - line_stride;
            ++row_count_;
        }
        GraphicWin::init(field_A14_ - menu_width, field_A18_ - menu_height,
                         menu_width, menu_height, nullptr, 0x1000020,
                         opening_window_snapshot_, nullptr, nullptr);

        field_A0_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
            ExpansionEnabled ? RedTintColorTable : BlueTintColorTable));
        show(0);
        static_cast<Buffer *>(this)->set_text_color(0x41, -1, 1, 1);
        static_cast<Buffer *>(this)->set_font(&font, nullptr, nullptr, nullptr);

        WinFlipClipped = 1;
        if ((GameDataStateStorage.fields_.game_preferences_ & 0x400) != 0) {
            wave.play();
        }

        for (int frame_intro = 0; frame_intro != 6; ++frame_intro) {
            const DWORD started = timeGetTime();
            flic.decode_stored_frame();
            int y = 0;
            for (uint32_t copy_row_intro = 0; copy_row_intro < row_count_; ++copy_row_intro) {
                const int height = static_cast<int>(buffer2_.dib_.bmiHeader.biHeight);
                const int width = static_cast<int>(buffer2_.dib_.bmiHeader.biWidth);
                buffer2_.copy(static_cast<Buffer *>(this), 0, 0, 0, y,
                              width, -height);
                y -= height;
                if (copy_row_intro == 0 && has_token) {
                    fill(0, y, width, 0x14, 9);
                    y += 0x14;
                }
            }
            on_update();
            while (static_cast<DWORD>(timeGetTime() - started) < 10) {
            }
        }

        title_rect.left = 0x0F;
        title_rect.top = 0;
        title_rect.right = 0x12C;
        title_rect.bottom = -line_stride;
        if (has_token) {
            static_cast<Buffer *>(this)->set_text_color(0x42, -1, 1, 1);
            const int top = title_rect.top + 0x1F;
            const int left = title_rect.left + 4;
            const int bottom = top + 0x1A - 2;
            const int second_top = top + 2;
            const int right = title_rect.right - 4;
            title_rect.left = left;
            title_rect.top = second_top;
            title_rect.right = right;
            title_rect.bottom = bottom;
            static_cast<Buffer *>(this)->write_cent_l(token, &title_rect,
                                                       static_cast<int>(strlen(token)));
            --row_count_;
            title_rect.left -= 4;
            title_rect.right += 4;
            title_rect.top = 0x14 - line_stride;
            title_rect.bottom = title_rect.top - line_stride;
        }

        for (uint32_t spot_row = 0; spot_row < row_count_; ++spot_row) {
            const int left = title_rect.left;
            const int top = title_rect.top;
            const int right = title_rect.right;
            const int bottom = title_rect.bottom;
            spot_.add(static_cast<int>(spot_row), 0, left, top,
                      right - left, bottom - top);
            title_rect.top = top - line_stride;
            title_rect.bottom = bottom - line_stride;
        }

        WinFlipClipped = 0;
        for (uint32_t draw_row = 0; draw_row < row_count_; ++draw_row) {
            draw_item(static_cast<int>(draw_row), draw_row == selected_row_ ? 1 : 0);
        }
        soft_update();
        set_modal(0, nullptr, nullptr);
        if (has_token) {
            ++row_count_;
        }
        for (int frame_outro = 0; frame_outro != 6; ++frame_outro) {
            const DWORD started = timeGetTime();
            flic.decode_stored_frame();
            int y = 0;
            for (uint32_t copy_row_outro = 0; copy_row_outro < row_count_; ++copy_row_outro) {
                const int height = static_cast<int>(buffer2_.dib_.bmiHeader.biHeight);
                const int width = static_cast<int>(buffer2_.dib_.bmiHeader.biWidth);
                buffer2_.copy(static_cast<Buffer *>(this), 0, 0, 0, y,
                              width, -height);
                y -= height;
                if (copy_row_outro == 0 && has_token) {
                    fill(0, y, width, 0x14, 9);
                    y += 0x14;
                }
            }
            on_update();
            while (static_cast<DWORD>(timeGetTime() - started) < 10) {
            }
        }

        spot_.shutdown();
        buffer_.close();
        opening_window_snapshot_->restore_clip();
        hide();

        if (menu_popup_ != nullptr && static_cast<int32_t>(selected_row_) >= 0) {
            Dialog &dialog = static_cast<Dialog &>(menu_popup_->dialogs());
            result = dialog.pos_to_id(static_cast<int>(selected_row_));
        } else {
            result = static_cast<int>(selected_row_);
        }
        flic.close();
    }
    return result;
}

// ORIGINAL: 0x004AE380 ?on_mouse_move@SetupWin@@QAEHHH@Z 0x004AE380-0x004AE3DA
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004ae380.cpp"], "reason": "Home SetupWin hover selection through typed Spot and selected-item fields"}
// symbol    ?on_mouse_move@SetupWin@@UAEXHH@Z
void SetupWin::on_mouse_move(int x, int y) {
    int position;
    const int hit_type = spot_.check(x, y, &position, 0);
    if (hit_type == -1) {
        return;
    }

    if (position == static_cast<int>(selected_row_)) {
        return;
    }

    draw_item(static_cast<int>(selected_row_), 0);
    selected_row_ = static_cast<uint32_t>(position);
    draw_item(position, 1);
    soft_update();
}

// ORIGINAL: 0x004AE4D0 ?draw_item@SetupWin@@QAEHHH@Z 0x004AE4D0-0x004AE757
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004ae4d0.cpp", "source:src/setupwin.h:SetupWin", "source:src/stringstruct.h:StringList", "source:src/game.h:opening_window"], "reason": "Restore the item-row painter, text cursor, and selected overlay through typed SetupWin members and a real StringList; the opening-window producer remains a separate storage migration."}
// TRIED: void return (no eax set anywhere in 647 bytes, so int->void); matches to #29/647 bytes. neg/sbb/and guard for the `this!=0` select before the first copy() call got closest; inlining the set_text_color color ternary into the call vs. a named local made no difference to where it first diverges.
int SetupWin::draw_item(int position, int selected) {
    if (position >= static_cast<int>(row_count_)) {
        return 0;
    }

    RECT row;
    spot_.get_rect(position, 0, &row);
    row.top += 31;
    row.bottom = row.top + 26;
    buffer_.set_clip(&row);
    buffer_.set_text_color(selected ? 0xCA : 0xCD, -1, 1, 1);

    buffer2_.copy(&buffer_, 0, 0, row.left - 15, row.top - 31,
                  buffer2_.dib_.bmiHeader.biWidth,
                  -buffer2_.dib_.bmiHeader.biHeight);

    row.top += 2;
    row.bottom -= 2;
    row.left += 12;
    row.right += 4;

    if (position >= 0 && item_texts_ != 0) {
        if (position < item_texts_->entry_count_) {
            item_texts_->current_ = item_texts_->head_;
            for (int i = position; i > 0 && item_texts_->current_ != 0; --i) {
                item_texts_->current_ = item_texts_->current_->next;
            }
            item_texts_->current_position_ = position;
        }
        StringStructEntry *entry = item_texts_->current_;
        char *name = item_texts_->head_ != 0 && entry != 0 &&
                     entry->string_payload() != 0 ? entry->string_payload()->text : 0;
        if (name != 0) {
            CharUpperA(name);
            char *pipe = strchr(name, '|');
            if (pipe != 0) *pipe = ' ';
            char *paren = strchr(name, '(');
            if (paren != 0 && paren > name) paren[-1] = 0;
            buffer_.write_cent_l(name, &row, static_cast<int>(strlen(name)));
        }
    }

    GraphicWin *opening = opening_window();
    if (selected && opening != 0 &&
        (WinScreenWidth == 800 || WinScreenWidth == 1024)) {
        int index = position < 0 ? 0 : position > 2 ? 2 : position;
        Buffer *overlay = overlay_buffers_[index];
        if (overlay != 0) {
            Buffer *destination = static_cast<Buffer *>(opening);
            const int x = overlay_x_[index];
            const int y = overlay_y_[index];
            const int width = overlay->dib_.bmiHeader.biWidth;
            const int height = -overlay->dib_.bmiHeader.biHeight;
            if (overlay->get_pixel(0, 0) == 9) {
                overlay->draw(destination, 9, x, y, 1, 1);
            } else {
                overlay->copy(destination, 0, 0, x, y, width, height);
            }
            opening->soft_update(x, y, width, height);
        }
    }
    buffer_.restore_clip();
    return 0;
}

// ORIGINAL: 0x004AD880 ??0SetupWin@@QAE@XZ 0x004AD880-0x004AD9C0;0x006595D0-0x0065960C
// RECOVERY_BINDING: v1 {"reason":"Recover the complete body in its genuine C++ owner, with real embedded members, shared virtual bases, lifecycle and parent operations. Compiler-generated overrides/adjustors replace executable-address or vbtable stand-ins; no current matching claim is granted.","evidence":["git:de91fffe:src/recovered/units/004ad880.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/ui-eight-owner-coverage.json","Original entry 0x004ad880; physical VC6 COFF emitted symbol ??0SetupWin@@QAE@XZ."]}
SetupWin::SetupWin() {
    overlay_buffers_[0] = 0;
    overlay_buffers_[1] = 0;
    overlay_buffers_[2] = 0;
    row_count_ = 0;
    selected_row_ = 0;
    opening_window_snapshot_ = opening_window();
    menu_popup_ = 0;
    field_A14_ = WinScreenWidth;
    field_A18_ = WinScreenHeight;
}

// SetupWin::close, 0x004AD9C0. The setup owner clears its per-window fields,
// closes its Buffer and Spot members, then drains the embedded StringList.
/*
Purpose: Clear SetupWin state and release its Buffer, Spot and embedded list.
// ORIGINAL: 0x004AD9C0 ?close@SetupWin@@QAEXXZ 0x004AD9C0-0x004ADA7B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x004AD9C0..0x004ADA7A is ?close@SetupWin@@QAEXXZ; inspect --callers finds 13 direct incoming calls in mapped newgame.cpp spans.", "Pinned operation order is selected_row, row_count, field_A14, field_A18, Buffer::close at +0xA44, Spot::shutdown at +0xFD4, embedded list cleanup at +0xFE0, and unconditional cursor reset at +0xFF4. These are existing SetupWin members.", "The +0xFE0 record is the declared embedded StringList field_FE0_; StringStructEntry/payload types and StringList::on_remove/remove_all are source-owned. `StringStruct::remove_all` uses the same head/next/payload callback and deleting sequence as the pinned loop, with a live entry_count_ reload each iteration.", "SetupWin::~SetupWin already uses field_FE0_.remove_all() after the same state/Buffer/Spot cleanup. This migration does not broaden SetupWin extent or invent the unused item_texts_ pointer as the list owner.", "Pinned standalone destructor 4AE790 restores SetupWin vptrs, performs close-state/Buffer/Spot/list cleanup, then destroys its actual members and GraphicWin base.", "All 13 mapped newgame calls to 4AD9C0 follow identical derived vptr restoration and precede member/base destruction: compiler-expanded destructor-body calls, not explicit pre-destruction closes.", "Pinned Popup close404900 and BasePop destructor4064D0 are distinct identities; terminal Popup close calls likewise belong to generated Popup destruction. Preserve config_game nonterminal close before object reuse.", "size_of_planet and custom_planet declare Popup before SetupWin; map_menu/top_menu declare SetupWin before Popup; config_game declares Popup before SetupWin. Reverse automatic destruction reproduces the observed owner order.", "Pinned size_of_planet success writes UnkBitfield1 before SetupWin destruction; config_game mode-bit2 success converges on common destruction. Automatic lifetimes cover both without duplicate terminal close calls."], "reason": "Recover SetupWin cleanup through its genuine members and ordinary destructor lifetime. The destructor invokes close once before compiler-owned member/base teardown; callers rely on automatic destruction rather than duplicate explicit closes. Original normal/error/exceptional cleanup and historical matching obligations remain; no new matching claim."}
// symbol    ?close@SetupWin@@QAEXXZ
*/
void SetupWin::close() {
    selected_row_ = 0;
    row_count_ = 0;
    field_A14_ = 0;
    field_A18_ = 0;
    buffer_.close();
    spot_.shutdown();

    if (field_FE0_.head_ != 0) {
        field_FE0_.remove_all();
    }
    field_FE0_.current_position_ = 0;
}



// ORIGINAL: 0x004AE790 ??1SetupWin@@QAE@XZ 0x004AE790-0x004AE99E;0x006596A0-0x006596F5
// RECOVERY_BINDING: v1 {"evidence": ["git:de91fffe:src/recovered/units/004ae790.cpp", "image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/ui-eight-owner-coverage.json", "Original entry 0x004ae790; physical VC6 COFF emitted symbol ??1SetupWin@@UAE@XZ.", "Pinned standalone destructor 4AE790 restores SetupWin vptrs, performs close-state/Buffer/Spot/list cleanup, then destroys its actual members and GraphicWin base.", "All 13 mapped newgame calls to 4AD9C0 follow identical derived vptr restoration and precede member/base destruction: compiler-expanded destructor-body calls, not explicit pre-destruction closes.", "Pinned Popup close404900 and BasePop destructor4064D0 are distinct identities; terminal Popup close calls likewise belong to generated Popup destruction. Preserve config_game nonterminal close before object reuse.", "size_of_planet and custom_planet declare Popup before SetupWin; map_menu/top_menu declare SetupWin before Popup; config_game declares Popup before SetupWin. Reverse automatic destruction reproduces the observed owner order.", "Pinned size_of_planet success writes UnkBitfield1 before SetupWin destruction; config_game mode-bit2 success converges on common destruction. Automatic lifetimes cover both without duplicate terminal close calls."], "reason": "Recover SetupWin cleanup through its genuine members and ordinary destructor lifetime. The destructor invokes close once before compiler-owned member/base teardown; callers rely on automatic destruction rather than duplicate explicit closes. Original normal/error/exceptional cleanup and historical matching obligations remain; no new matching claim."}
// symbol    ??1SetupWin@@UAE@XZ
SetupWin::~SetupWin() {
    close();
}

// ORIGINAL: 0x004AE3E0 ?on_key_click@SetupWin@@QAEHHH@Z 0x004AE3E0-0x004AE49D
// RECOVERY_BINDING: v1 {"reason":"Recover the complete body in its genuine C++ owner, with real embedded members, shared virtual bases, lifecycle and parent operations. Compiler-generated overrides/adjustors replace executable-address or vbtable stand-ins; no current matching claim is granted.","evidence":["git:de91fffe:src/unrecovered/004ae3e0.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/ui-eight-owner-coverage.json","Original entry 0x004ae3e0; physical VC6 COFF emitted symbol ?on_key_click@SetupWin@@UAEHHH@Z."]}
// symbol    ?on_key_click@SetupWin@@UAEHHH@Z
int SetupWin::on_key_click(int, int key) {
    switch (key) {
    case 0x0d:
        release_modal();
        return 1;
    case 0x1b:
        selected_row_ = static_cast<uint32_t>(-1);
        release_modal();
        return 1;
    case 0x26: {
        const int current = static_cast<int>(selected_row_);
        const int next = current == 0
            ? static_cast<int>(row_count_) - 1 : current - 1;
        draw_item(current, 0);
        selected_row_ = static_cast<uint32_t>(next);
        draw_item(next, 1);
        soft_update();
        return 1;
    }
    case 0x28: {
        const int current = static_cast<int>(selected_row_);
        const int next = current == static_cast<int>(row_count_) - 1
            ? 0 : current + 1;
        draw_item(current, 0);
        selected_row_ = static_cast<uint32_t>(next);
        draw_item(next, 1);
        soft_update();
        return 1;
    }
    default:
        return 0;
    }
}
// ORIGINAL: 0x004AE9A0 ??_GSetupWin@@WEEE@AEPAXI@Z 0x004AE9A0-0x004AE9AB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf.", "Pinned vftable slot 0: 0x0066D8E0 -> 0x004AE9A0.", "the SetupWin constructor stores secondary Buffer table 0x0066D8E0 at object+0x444", "The original 11-byte adjustor corrects the earlier mistaken `??3SetupWin@@SAXPAXI@Z` interpretation: it subtracts 0x444 from ECX and tail-transfers to primary scalar-deleting entry 0x004AE760; the corrected `WEEE` signature describes the compiler's adjusting-deleting entry.", "The historical disassembly note records that the primary path returns with ret 4, tests deletion flag bit 0, and invokes scalar operator delete only when that bit is set.", "The historical demangling note identifies adjustment 1092, equal to 0x444; the adjustor does not otherwise access the stack or change the receiver view.", "Source owner: src/setupwin.cpp; generated entry name is compiler-observed as ??_ESetupWin@@WEEE@AEPAXI@Z."], "reason": "Retain the original SetupWin secondary-Buffer deleting-adjustor identity as a compiler-generated entry owned by the real SetupWin destructor and GraphicWin hierarchy. The pinned secondary view at 0x0066D8E0, slot 0, targets 0x004AE9A0; this entry adjusts the Buffer receiver by 0x444 and reaches the primary scalar-deleting destructor, whose original path tests flag bit 0 before releasing storage. The current compiler-emitted symbol is recorded from the retained owner COFF, not handwritten."}
// symbol    ??_ESetupWin@@WEEE@AEPAXI@Z
// kind      game
