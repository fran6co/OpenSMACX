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
#include "stdafx.h"
#include "original_seam.h"
#include "editbox.h"
#include "graphicwin.h"
#include "time.h"

// The original SDK import directory identifies this key-state dependency.
// RECOVERY_IMPORT: v1 __imp__GetAsyncKeyState@4 USER32.dll name:GetAsyncKeyState

// Keep the original out-of-line stop boundary here. Other timer owners rely
// on the inline definition in time.h for their own cleanup chains.
static void __fastcall stop_edit_timer(Time *timer) {
    timer->stop();
}

// 615DF0. Calls slot 62 virtually, not slot 12 directly.
// ORIGINAL: 0x00615DF0 ?on_dialog_focus@EditBox@@QAEXH@Z 0x00615DF0-0x00615E31 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00615df0.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?vslot_51@EditBox@@UAEXH@Z
void EditBox::vslot_51(int focused) {
    if (focused) {
        time_.start();
    } else {
        stop_edit_timer(&time_);
        field_B48_ = 0;
    }
    redraw();
}

// 615E60. Genuine owner globals already live in win.cpp.
// ORIGINAL: 0x00615E60 ?on_left_down@EditBox@@QAEXHH@Z 0x00615E60-0x00615EB5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00615e60.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?vslot_20@EditBox@@UAEXHH@Z
void EditBox::vslot_20(int x, int) {
    if (WinPointerOwner3 == 0) {
        WinPointerOwner3 = this;
        WinPointerOwner4 = 0;
    }
    field_B44_ = text_position(x);
    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0) {
        field_B40_ = field_B44_;
    }
    redraw();
}

// 615EC0. Keep the virtual release hook before clearing the second owner.
// ORIGINAL: 0x00615EC0 ?on_left_up@EditBox@@QAEXHH@Z 0x00615EC0-0x00615F0D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00615ec0.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?vslot_21@EditBox@@UAEXHH@Z
void EditBox::vslot_21(int x, int) {
    if (WinPointerOwner3 == this) {
        WinPointerOwner3 = 0;
        vslot_04();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
    field_B44_ = text_position(x);
    redraw();
}

// 615F10. This is the TWO-argument slot 17, not slot 16's mouse router.
// ORIGINAL: 0x00615F10 ?on_mouse_move@EditBox@@QAEXHH@Z 0x00615F10-0x00615F4E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00615f10.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?vslot_17@EditBox@@UAEXHH@Z
void EditBox::vslot_17(int x, int) {
    if (WinPointerOwner3 == this ||
        (WinPointerOwner4 == this && WinPointerOwner3 != 0 && WinPointerOwner4 != 0)) {
        field_B44_ = text_position(x);
        redraw();
    }
}

// 616140. Qualified base calls are required: ordinary virtual calls recurse.
// ORIGINAL: 0x00616140 ?pass_dialog_focus@EditBox@@QAEXXZ 0x00616140-0x00616196
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00616140.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?pass_dialog_focus@EditBox@@UAEXXZ
void EditBox::pass_dialog_focus() {
    Win *parent = win_parent_;
    if (parent == 0) return;
    if ((field_B3C_ & 1) != 0 &&
        parent->list_.tail_ == parent->list_.count_ - 1) {
        if (parent->list_.count_ - 1 >= 0) {
            parent->list_.tail_ = 0;
            parent->list_.current_ = parent->list_.head_;
        }
        win_parent_->Win::pass_dialog_focus();
        return;
    }
    Win::pass_dialog_focus();
}

// 616030. Empty selected text returns zero, NOT the current scroll position.
// The last comparison is against B44, not the x argument or string length.
// ORIGINAL: 0x00616030 ?text_position@EditBox@@QAEXH@Z 0x00616030-0x00616135
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00616030.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?text_position@EditBox@@QAEHH@Z
int EditBox::text_position(int x) {
    if (x < static_cast<int>(field_B34_)) return static_cast<int>(field_B18_);
    x -= static_cast<int>(field_B34_);
    char masked[256];
    char *text;
    if (field_B3C_ & 2) {
        masked[0] = 0;
        strcat(masked, text_);
        for (char *p = masked; *p; ++p) *p = '*';
        text = masked;
    } else {
        text = text_ + static_cast<int>(field_B18_);
    }
    if (*text == 0) return 0;
    const int length = static_cast<int>(strlen(text));
    int count = 1;
    if (length > 1) {
        do {
            if (Buffer::text_width(text, count) > x) break;
            ++count;
        } while (count < length);
    }
    if (count < static_cast<int>(field_B44_))
        return static_cast<int>(field_B18_) + count - 1;
    return static_cast<int>(field_B18_) + count;
}

// 615F50. Preserve the second mask read, strict width inequality, width read
// before the text-width call, and padding read after it.
// ORIGINAL: 0x00615F50 ?text_fits@EditBox@@QAEHXZ 0x00615F50-0x00616029
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00615f50.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?text_fits@EditBox@@QAEHXZ
int EditBox::text_fits() {
    char masked[256];
    if (field_B3C_ & 2) {
        masked[0] = 0;
        strcat(masked, text_);
        for (char *p = masked; *p; ++p) *p = '*';
        if (field_B3C_ & 2) {
            int available = dib_.bmiHeader.biWidth;
            int width = Buffer::text_width(masked,
                static_cast<int>(field_B44_ - field_B18_));
            return width < available - static_cast<int>(field_B34_) * 2;
        }
    }
    int available = dib_.bmiHeader.biWidth;
    int width = Buffer::text_width(text_ + static_cast<int>(field_B18_),
        static_cast<int>(field_B44_ - field_B18_));
    return width < available - static_cast<int>(field_B34_) * 2;
}

// 615630. No invented bounds clamp: preserve the original selection cases.
// ORIGINAL: 0x00615630 ?on_key_click@EditBox@@QAEHHH@Z 0x00615630-0x00615839
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00615630.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?vslot_25@EditBox@@UAEHHH@Z
int EditBox::vslot_25(int character, int) {
    if (character >= 0 && character < 0x20) return 0;
    if (strlen(text_) == field_B14_ && field_B40_ == field_B44_) {
        MessageBeep(0);
        return 1;
    }
    char ch = static_cast<char>(character);
    if (field_B40_ == field_B44_) {
        if (strlen(text_) == 255) {
            MessageBeep(0);
            return 1;
        }
        int position = static_cast<int>(field_B44_);
        if (text_[position]) {
            memmove(text_ + position + 1, text_ + position, strlen(text_ + position) + 1);
        } else {
            text_[position + 1] = 0;
        }
        text_[position] = ch;
        field_B44_ = position + 1;
        ++field_B40_;
        // Only collapsed insertion reaches 615803's text_fits loop.
        // Both nonempty-selection branches jump straight to 615824/redraw;
        // the preserved seed incorrectly placed this loop after all branches.
        while (!text_fits()) ++field_B18_;
    } else if (static_cast<int>(field_B40_) < static_cast<int>(field_B44_)) {
        text_[static_cast<int>(field_B40_)] = ch;
        if (static_cast<int>(field_B44_ - field_B40_) > 1) {
            memmove(text_ + static_cast<int>(field_B40_) + 1,
                text_ + static_cast<int>(field_B44_),
                strlen(text_ + static_cast<int>(field_B44_)) + 1);
        }
        field_B44_ = field_B40_ + 1;
        ++field_B40_;
    } else {
        text_[static_cast<int>(field_B44_)] = ch;
        if (static_cast<int>(field_B40_ - field_B44_) > 1) {
            memmove(text_ + static_cast<int>(field_B44_) + 1,
                text_ + static_cast<int>(field_B40_),
                strlen(text_ + static_cast<int>(field_B40_)) + 1);
        }
        ++field_B44_;
        field_B40_ = field_B44_;
    }
    redraw();
    return 1;
}

// Key modifiers are encoded in the upper word; the low word selects the
// original Home/End, arrow, Backspace and Delete branches.
// ORIGINAL: 0x00615840 ?on_key_down@EditBox@@QAEHH@Z 0x00615840-0x00615D73
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00615840.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?vslot_26@EditBox@@UAEHH@Z
int EditBox::vslot_26(int key_value) {
    unsigned int key = static_cast<unsigned int>(key_value);
    if (key == 9) return 0;
    field_B38_ = 0;
    if (key & 0x10000) field_B38_ = 1;
    if (key & 0x20000) field_B38_ |= 2;
    unsigned int low = key & 0xffff;
    if (low - 8 > 0x5e) return 0;
    switch (low) {
    case 8: {
        if (field_B38_ != 0) { MessageBeep(0); return 1; }
        int cursor = static_cast<int>(field_B44_);
        int mark = static_cast<int>(field_B40_);
        if (mark == cursor) {
            if (cursor == 0) { MessageBeep(0); return 1; }
            if (text_[cursor] == 0) text_[cursor - 1] = 0;
            else memmove(text_ + cursor - 1, text_ + cursor, strlen(text_ + cursor) + 1);
        } else if (mark < cursor) {
            memmove(text_ + mark, text_ + cursor, strlen(text_ + cursor) + 1);
            field_B44_ = mark + 1;
        } else {
            memmove(text_ + cursor, text_ + mark, strlen(text_ + mark) + 1);
            field_B44_ = cursor + 1;
        }
        --field_B44_;
        field_B40_ = field_B44_;
        if (static_cast<int>(field_B44_) < static_cast<int>(field_B18_)) field_B18_ = field_B44_;
        redraw();
        return 1;
    }
    case 0x23: {
        field_B44_ = strlen(text_);
        if ((field_B38_ & 1) == 0) field_B40_ = field_B44_;
        char *p = text_ + static_cast<int>(field_B44_);
        if (p > text_) {
            do {
                int available = dib_.bmiHeader.biWidth;
                int width = Buffer::text_width(p);
                available -= static_cast<int>(field_B34_) * 2;
                if (width >= available) break;
                --p;
            } while (p > text_);
        }
        if (p != text_) ++p;
        field_B18_ = static_cast<int>(p - text_);
        redraw();
        return 1;
    }
    case 0x24:
        field_B44_ = 0;
        if ((field_B38_ & 1) == 0) field_B40_ = 0;
        field_B18_ = 0;
        redraw();
        return 1;
    case 0x25:
    case 0x64: {
        int cursor = static_cast<int>(field_B44_);
        if (cursor == 0) {
            if (field_B38_ & 1) return 1;
            field_B40_ = 0;
            redraw();
            return 1;
        }
        unsigned int flags = field_B38_;
        int delta = 1;
        if ((flags & 2) && text_[cursor - 1] != ' ') {
            char *p = text_ + cursor - 1;
            while (p > text_ && *p != ' ') --p;
            delta = static_cast<int>(text_ + cursor - p);
        }
        int next = cursor - delta;
        field_B44_ = next;
        if (next < static_cast<int>(field_B18_)) field_B18_ = next;
        if ((flags & 1) == 0) field_B40_ = next;
        redraw();
        return 1;
    }
    case 0x27:
    case 0x66: {
        unsigned int flags = static_cast<unsigned char>(field_B38_);
        int cursor = static_cast<int>(field_B44_);
        int delta = 1;
        if ((flags & 2) && text_[cursor] != ' ') {
            char *found = strchr(text_ + cursor, ' ');
            delta = found ? static_cast<int>(found - (text_ + cursor)) + 1
                          : static_cast<int>(strlen(text_ + cursor));
        }
        int next = cursor + delta;
        field_B44_ = next;
        if (text_[next - 1] == 0) {
            if (strlen(text_) == field_B14_) { MessageBeep(0); return 1; }
            text_[next - 1] = ' ';
            text_[next] = 0;
        }
        while (!text_fits()) ++field_B18_;
        // 615985 rereads this flag after the text_fits calls.
        if ((field_B38_ & 1) == 0) field_B40_ = field_B44_;
        redraw();
        return 1;
    }
    case 0x2e: {
        if (field_B38_ != 0) { MessageBeep(0); return 1; }
        int cursor = static_cast<int>(field_B44_);
        int mark = static_cast<int>(field_B40_);
        if (mark == cursor) {
            if (text_[cursor] == 0) { MessageBeep(0); return 1; }
            memmove(text_ + cursor, text_ + cursor + 1, strlen(text_ + cursor + 1) + 1);
        } else if (mark > cursor) {
            memmove(text_ + cursor, text_ + mark, strlen(text_ + mark) + 1);
        } else {
            memmove(text_ + mark, text_ + cursor, strlen(text_ + cursor) + 1);
            field_B44_ = mark;
        }
        field_B40_ = field_B44_;
        if (static_cast<int>(field_B44_) < static_cast<int>(field_B18_)) field_B18_ = field_B44_;
        redraw();
        return 1;
    }
    default:
        return 0;
    }
}

// 615270. Do not cache the mask flag across drawing calls; original reloads
// it at each branch. Rendering is intentionally inherited virtual redraw(),
// whose eventual slot-12 callback reaches this handler.
// ORIGINAL: 0x00615270 ?on_redraw@EditBox@@QAEXXZ 0x00615270-0x00615627
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00615270.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?vslot_12@EditBox@@UAEXXZ
void EditBox::vslot_12() {
    char masked[256];
    if (field_B3C_ & 2) {
        masked[0] = 0;
        strcat(masked, text_);
        for (char *p = masked; *p; ++p) *p = '*';
    }
    Buffer::set_clip(&rect2_);
    GraphicWin::fill(static_cast<int>(field_B1C_));
    RECT area = rect2_;
    for (int i = 0; i < static_cast<int>(field_B30_); ++i) {
        Buffer::box(&area, static_cast<int>(field_B28_), static_cast<int>(field_B2C_));
        ++area.left;
        --area.right;
        ++area.top;
        --area.bottom;
    }
    int border = static_cast<int>(field_B30_);
    int padding = static_cast<int>(field_B34_);
    area.left -= border - padding;
    area.right += border - padding;
    area.top -= border;
    area.bottom += border;
    int line_height = Buffer::text_line_height();
    int center = -(line_height + dib_.bmiHeader.biHeight) >> 1;
    if (field_B48_) {
        char *text;
        int length;
        if (field_B3C_ & 2) {
            length = static_cast<int>(field_B44_ - field_B18_);
            text = masked;
        } else {
            int first = static_cast<int>(field_B18_);
            length = static_cast<int>(field_B44_) - first;
            text = text_ + first;
        }
        int x = Buffer::text_width(text, length);
        x += static_cast<int>(field_B34_);
        int color = static_cast<int>(color_val_1_);
        int height = Buffer::text_line_height();
        Buffer::vline(x, center, height + center, color);
    }
    Buffer::set_clip(&area);
    char *visible = (field_B3C_ & 2) ? masked : text_ + static_cast<int>(field_B18_);
    if (visible) Buffer::write_l(visible, &area, strlen(visible));

    int mark = static_cast<int>(field_B40_);
    int cursor = static_cast<int>(field_B44_);
    if (mark != cursor) {
        char *selected;
        int length;
        int x;
        if (mark > cursor) {
            if (field_B3C_ & 2) {
                int first = static_cast<int>(field_B18_);
                x = Buffer::text_width(masked, cursor - first);
                x += static_cast<int>(field_B34_);
                length = static_cast<int>(field_B40_ - field_B44_);
                selected = masked;
            } else {
                int first = static_cast<int>(field_B18_);
                x = Buffer::text_width(text_ + first, cursor - first);
                x += static_cast<int>(field_B34_);
                selected = text_ + static_cast<int>(field_B44_);
                length = static_cast<int>(field_B40_ - field_B44_);
            }
        } else {
            int first = static_cast<int>(field_B18_);
            int start = mark > first ? mark : first;
            if (field_B3C_ & 2) {
                x = Buffer::text_width(masked, start - first);
                x += static_cast<int>(field_B34_);
                length = static_cast<int>(field_B44_ - field_B40_);
                selected = masked;
            } else {
                x = Buffer::text_width(text_ + first, start - first);
                x += static_cast<int>(field_B34_);
                selected = text_ + start;
                length = static_cast<int>(field_B44_ - field_B40_);
            }
        }
        int width = Buffer::text_width(selected, length);
        int foreground = static_cast<int>(field_B24_);
        if (foreground == -1) foreground = static_cast<int>(color_val_1_);
        int background = static_cast<int>(field_B20_);
        if (background == -1) background = static_cast<int>(field_B1C_);
        int original_background = static_cast<int>(field_B1C_);
        int temporary_color = foreground + 1;
        int height = Buffer::text_line_height();
        Buffer::change_color(x, center, width, height, original_background, temporary_color);
        int original_foreground = static_cast<int>(color_val_1_);
        height = Buffer::text_line_height();
        Buffer::change_color(x, center, width, height, original_foreground, background);
        height = Buffer::text_line_height();
        Buffer::change_color(x, center, width, height, temporary_color, foreground);
    }
}

/*
 * THE SEVEN GLOBALS EditBox::close REPUBLISHES, and why they are named after
 * their DESTINATIONS rather than after anything they mean.
 *
 * Every member they land in is `field_BXX_` - the class carries no recovered
 * name for any of them - so a name claiming a purpose would be invented. What
 * IS known is which field each one feeds, and that all seven are read at close
 * time to republish per-instance defaults, the same role EditGroupDefault
 * plays for EditGroup.
 *
 * 0x006971DC-0x006971F8 is one contiguous run in .data; 0x009B90B8 and
 * 0x009B90BC sit apart from it, in the zero-filled range.
 *
 * Independent DWORD reads at 614EA4..614EEE establish each four-byte extent.
 * These bindings identify the original targets, not just masked operands.
 */
// RECOVERY_OBJECT: v2 0x006971DC 0x006971E0 ?EditBoxDefaultB1C@@3IA 0x00614E50
uint32_t EditBoxDefaultB1C = 255;
// RECOVERY_OBJECT: v2 0x006971F8 0x006971FC ?EditBoxDefaultB20@@3IA 0x00614E50
uint32_t EditBoxDefaultB20 = -1;
// RECOVERY_OBJECT: v2 0x006971F4 0x006971F8 ?EditBoxDefaultB24@@3IA 0x00614E50
uint32_t EditBoxDefaultB24 = -1;
// RECOVERY_OBJECT: v2 0x009B90B8 0x009B90BC ?EditBoxDefaultB28@@3IA 0x00614E50
uint32_t EditBoxDefaultB28;
// RECOVERY_OBJECT: v2 0x009B90BC 0x009B90C0 ?EditBoxDefaultB2C@@3IA 0x00614E50
uint32_t EditBoxDefaultB2C;
// RECOVERY_OBJECT: v2 0x006971E0 0x006971E4 ?EditBoxDefaultB30@@3IA 0x00614E50
uint32_t EditBoxDefaultB30 = 1;
// RECOVERY_OBJECT: v2 0x006971E4 0x006971E8 ?EditBoxDefaultB34@@3IA 0x00614E50
uint32_t EditBoxDefaultB34 = 4;


/*
Purpose: Construct the GraphicWin base and trailing Time member, then publish
         the EditBox field defaults. C++ owns base/member lifetime and tables.
// ORIGINAL: 0x00614E50 ??0EditBox@@QAE@XZ 0x00614E50-0x00614F2B
// symbol    ??0EditBox@@QAE@XZ
// size      220 bytes
// prototype void (__thiscall ??0EditBox@@QAE@XZ)(EditBox* this)
// callers   2   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x006161D0
Return Value: n/a

The image's call 5D4CF0 constructs GraphicWin on the unadjusted receiver.
Its Time constructor call receives this+B4C. Neither is an explicit source
call on an already constructed member.
*/
EditBox::EditBox() {
    text_[0] = 0;
    field_B14_ = 0xA;
    field_B28_ = EditBoxDefaultB28;
    field_B2C_ = EditBoxDefaultB2C;
    field_B30_ = EditBoxDefaultB30;
    field_B34_ = EditBoxDefaultB34;
    field_B1C_ = EditBoxDefaultB1C;
    field_B20_ = EditBoxDefaultB20;
    field_B24_ = EditBoxDefaultB24;
    field_B38_ = 0;
    field_B40_ = 0;
    field_B44_ = 0;
    field_B48_ = 0;
    field_B18_ = 0;
    field_B3C_ = 0;
}

// ORIGINAL: 0x00408010 ??1EditBox@@QAE@XZ 0x00408010-0x00408077;0x00650CF0-0x00650D10 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00408010.cpp", "Original constructor/destructor direct-call receivers and unwind stages"], "reason": "408010 closes EditBox then destroys its Time member and nonvirtual GraphicWin base; normal and unwind paths agree."}
// symbol    ??1EditBox@@UAE@XZ
EditBox::~EditBox() {
    close();
}

/*
Purpose: Reset the edit box - clear its state, republish the seven defaults,
         then close the trailing Time and the GraphicWin prefix.
The historical forwarding notes below predate the separate timer lifecycle TU;
this definition now calls the actual Time member.
// ORIGINAL: 0x00614F30 ?close@EditBox@@QAEXXZ 0x00614F30-0x00614FD1 BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/00614f30.cpp, whose claim proved the ARTIFACT reproduced while the program held a pending_bodies forwarder. src/editbox.cpp did not exist - EditBox was header-only - so this file and its CMakeLists entry are new.
// LEVER (2026-09-03): time_close_call, not `->close()` on the Time at 0xb4c.
//        The image CALLS 0x00616780 here (0x00614FC3) where time.h's
//        in-class `MEASURED close` otherwise expands at this site too -
//        61 compiled instructions against the image's 31, 2/31 agreeing.
//        Same forwarder flatbutton.cpp uses for its own Time member, and
//        netmsg.cpp now carries a static copy of it as well.
// symbol    ?close@EditBox@@QAEXXZ
// size      161 bytes
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00616780
Return Value: n/a
Status: Complete
*/


void EditBox::close() {
    text_[0] = 0;
    field_B14_ = 0xa;
    field_B28_ = EditBoxDefaultB28;
    field_B2C_ = EditBoxDefaultB2C;
    field_B30_ = EditBoxDefaultB30;
    field_B34_ = EditBoxDefaultB34;
    field_B1C_ = EditBoxDefaultB1C;
    field_B20_ = EditBoxDefaultB20;
    field_B24_ = EditBoxDefaultB24;
    field_B38_ = 0;
    field_B40_ = 0;
    field_B44_ = 0;
    field_B48_ = 0;
    field_B18_ = 0;
    field_B3C_ = 0;

    time_.close();
    GraphicWin::close();
}
/*
Purpose: Does this window hold the dialog focus, and its partner too?

             mov esi,ecx / call Win::is_dialog_focus / test eax,eax / je no
             mov ecx,[esi+0xc4] / test ecx,ecx / je yes
             call Win::is_dialog_focus / test eax,eax / je no
             yes: mov eax,1 / ret        no: xor eax,eax / ret

         The second call is made on the window at field 0xc4 - which
         src/win.h names `win_parent_` - not on `this` again; ECX is reloaded
         from it just before. A NULL parent is not a failure: the answer is
         yes.

// ORIGINAL: 0x006161A0 ?UNK2@EditBox@@QAEXXZ 0x006161A0-0x006161CA
// symbol    ?leaf_006161a0_redirect@@YIHPAX0@Z
// TRIED (2026-08-26, deleting win_is_dialog_focus_redirect and calling
// Win::is_dialog_focus directly): 2/17 -> 10/17 agreeing, 20 -> 18 compiled
// instructions. What remains is the wrapper's own tail: ours ends
// `push 1 / pop eax` where the image has `mov eax,1`, and the two early
// `je` distances differ by that one instruction. Restructuring to the
// image's exact arm order (null-parent arm returning 1 as its own `if`)
// REGRESSED to 4/17 with 19 instructions under /O2 - reverted. Plateau.
// size      42 bytes
// prototype void (__thiscall ?UNK2@EditBox@@QAEXXZ)(EditBox* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F2CA0
Return Value: 1 when both hold focus, 0 otherwise
Status: Complete
*/
int __fastcall leaf_006161a0_redirect(void *self, void *) {
    if (reinterpret_cast<Win *>(self)->is_dialog_focus() == 0) {
        return 0;
    }
    Win *const parent = *reinterpret_cast<Win *const *>(
        static_cast<uint8_t *>(self) + 0xC4);
    if (parent != nullptr && parent->is_dialog_focus() == 0) {
        return 0;
    }
    return 1;
}
