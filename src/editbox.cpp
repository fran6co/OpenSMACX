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
// ORIGINAL: 0x004083A0 ??_GEditBox@@WEEE@AEPAXI@Z 0x004083A0-0x004083AB FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original ??_GEditBox@@WEEE@AEPAXI@Z at 0x004083a0 is an 11-byte secondary scalar deleting adjustor; it subtracts 0x444 from ECX and tail-jumps to the complete-object EditBox scalar deleting entry without touching stack or flags.", "Historical seed correction: the earlier `??3` deallocation interpretation was refuted; the original is a `??_G` scalar-deleting adjustor. `WEEE` demangles to adjustor{1092}, equal to 0x444.", "Pinned EditBox constructor at 0x00614e8a stores Buffer-view vftable 0x0066a030 at this+0x444; the source owner is the typed EditBox GraphicWin/Buffer multiple-inheritance view.", "Current src/editbox.cpp COFF Buffer-view vftable first slot selects generated ??_EEditBox@@WEEE@AEPAXI@Z; its body performs the same 0x444 adjustment and preserves the flag argument.", "Current EditBox COFF weak external ??_EEditBox@@UAEPAXI@Z records search mode 2 (library search) and fallback target ??_GEditBox@@UAEPAXI@Z; this linked image resolves both primary labels to the same physical class deleting body. The object fallback and linked address are per-build evidence, not a general E/G equivalence or owner-selection rule.", "That measured current primary body calls EditBox::~EditBox, tests flags bit 0, calls operator delete only when bit 0 is set, returns the saved object pointer, and uses ret 4; this is the per-class measured E-to-G scalar path, not a general vector/scalar equivalence.", "The removed transitional C++ names were ?adjust_this_edit_box@@YIPAXPAX0I@Z and ?scalar_delete_edit_box@@YIPAXPAX0I@Z; no callers exist outside deleting_thunks.cpp/.h, and the current generated Buffer vftable uses the class-owned E adjustor instead.", "Historical seed metadata noted flags hidden;sp_ready;purged_ok, zero direct callers, and no direct call targets. These remain historical observations only; they do not establish virtual-dispatch completeness or acceptance.", "The old secondary record had no BYTE_EXACT or SEMANTIC claim. Its original identity, full span, kind and parsed history are moved unchanged; no matching claim is granted."], "reason": "Bind the pinned secondary scalar deleting identity for EditBox to its measured genuine Buffer-view generated adjustor. Keep the original G identity and point its current symbol fact at the owner-emitted E adjustor; the per-object weak fallback and bit-0 path above establish why this exact current implementation preserves the original operation."}
// symbol    ??_EEditBox@@WEEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x00408080 ??_GEditBox@@UAEPAXI@Z 0x00408080-0x004080A1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00408080-0x004080A1", "Pinned class constructor ??0EditBox@@QAE@XZ at 0x00614E50 stores original class/view table VA 0x66a038 at 0x00614E84 (mov dword ptr [esi], 0x66a038); the current genuine owner vftable ??_7EditBox@@6BWin@@@ selects generated ??_GEditBox@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EEditBox@@UAEPAXI@Z (mode 2).", "Pinned ??_GEditBox@@UAEPAXI@Z at 0x00408080 calls the source-mapped ??1EditBox@@QAE@XZ at 0x00408010, then tests byte ptr [ebp + 8], 1; je 0x40809a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/editbox.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original EditBox scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GEditBox@@UAEPAXI@Z
// kind      game
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

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00615E40 ?caret_callback@EditBox@@QAEXH@Z 0x00615E40-0x00615E5F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned disassembly 0x00615E40 reads its only receiver from [esp+4] and ends with a tail jump through the receiver's redraw slot", "git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00614fe0.cpp passes EditBox this as Time's integer callback payload", "src/editbox.h field_B48_ is the caret visibility state"], "reason": "Treat the timer entry as a free callback taking an EditBox pointer payload, not a member with an invented implicit receiver."}
// symbol    ?edit_box_caret_callback@@YAXH@Z
void __cdecl edit_box_caret_callback(int edit_box_address) {
    EditBox *const box = reinterpret_cast<EditBox *>(edit_box_address);
    box->field_B48_ = box->field_B48_ == 0;
    box->redraw();
}

// 615DF0. Calls slot 62 virtually, not slot 12 directly.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00615DF0 ?on_dialog_focus@EditBox@@QAEXH@Z 0x00615DF0-0x00615E31 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00615df0.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?on_dialog_focus@EditBox@@UAEXH@Z
void EditBox::on_dialog_focus(int focused) {
    if (focused) {
        time_.start();
    } else {
        stop_edit_timer(&time_);
        field_B48_ = 0;
    }
    redraw();
}

// 615E60. Genuine owner globals already live in win.cpp.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00615E60 ?on_left_down@EditBox@@QAEXHH@Z 0x00615E60-0x00615EB5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00615e60.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?on_left_down@EditBox@@UAEXHH@Z
void EditBox::on_left_down(int x, int) {
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
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00615EC0 ?on_left_up@EditBox@@QAEXHH@Z 0x00615EC0-0x00615F0D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00615ec0.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?on_left_up@EditBox@@UAEXHH@Z
void EditBox::on_left_up(int x, int) {
    if (WinPointerOwner3 == this) {
        WinPointerOwner3 = 0;
        on_lose_mouse_capture();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
    field_B44_ = text_position(x);
    redraw();
}

// 615F10. This is the TWO-argument slot 17, not slot 16's mouse router.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00615F10 ?on_mouse_move@EditBox@@QAEXHH@Z 0x00615F10-0x00615F4E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00615f10.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?on_mouse_move@EditBox@@UAEXHH@Z
void EditBox::on_mouse_move(int x, int) {
    if (WinPointerOwner3 == this ||
        (WinPointerOwner4 == this && WinPointerOwner3 != 0 && WinPointerOwner4 != 0)) {
        field_B44_ = text_position(x);
        redraw();
    }
}

// 616140. Qualified base calls are required: ordinary virtual calls recurse.
// Superseded seed deleted; seed paths below are historical Git provenance.
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
// Superseded seed deleted; seed paths below are historical Git provenance.
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
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00615f50.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// TRIED: plain `strcat(buf, ...)` compiles to an inlined `repne scasb`/copy sequence, not the original's `call _strcat`; `#pragma function(strcat)` restores the call. Landed at 82.4% mnemonic similarity (172 vs 217 bytes): the original duplicates its `text_width` + threshold-compare tail in BOTH branches (masked and unmasked text), but this source's identical trailing 3 statements in each branch let VC6 tail-merge them into one shared block with a `jmp`, which the original does not do even though its two tails are mnemonic-for-mnemonic identical too. Tried: a shared vs per-branch `threshold`/`base` local, folding the compare inline vs a named `threshold` variable - the merge persisted across all of them.
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
// Deleted seed provenance: git:25c0645e742c23f149e0895c391a9ba79ad744cb:src/unrecovered/00615630.cpp
// symbol    ?on_key_click@EditBox@@UAEHHH@Z
int EditBox::on_key_click(int character, int) {
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
// Deleted seed provenance: git:25c0645e742c23f149e0895c391a9ba79ad744cb:src/unrecovered/00615840.cpp
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
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00615270 ?on_redraw@EditBox@@QAEXXZ 0x00615270-0x00615627
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00615270.cpp", "Pinned original EditBox table66A038 and direct call/branch evidence"], "reason": "EditBox owns this table override or its direct text helper: pinned primary table66A038 slots12,17,20,21,25,26,51,64 and original direct-call receivers establish ownership. Use existing Win slot signatures without changing their order; text_position returns the EAX index consumed by callers despite the catalogue void label."}
// symbol    ?on_redraw@EditBox@@UAEXXZ
void EditBox::on_redraw() {
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

// The initializer reads these four consecutive colour defaults. The first
// occupies zero-filled storage; the others are initialized image data.
// RECOVERY_OBJECT: v2 0x009B90C0 0x009B90C4 ?EditBoxTextColor1@@3HA 0x00614FE0
int EditBoxTextColor1;
// RECOVERY_OBJECT: v2 0x006971E8 0x006971EC ?EditBoxTextColor2@@3HA 0x00614FE0
int EditBoxTextColor2 = -1;
// RECOVERY_OBJECT: v2 0x006971EC 0x006971F0 ?EditBoxTextColor3@@3HA 0x00614FE0
int EditBoxTextColor3 = 1;
// RECOVERY_OBJECT: v2 0x006971F0 0x006971F4 ?EditBoxTextColor4@@3HA 0x00614FE0
int EditBoxTextColor4 = 1;

// ORIGINAL: 0x00614FE0 ?init@EditBox@@QAEHHHHHPAUWin@@H@Z 0x00614FE0-0x00615179
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00614fe0.cpp", "EditBox member offsets in src/editbox.h", "Buffer::markup_enabled_ lies at GraphicWin+0x964", "Original image values at 0x006971E8-0x006971F3"], "reason": "Initialize the real editor, base window, text buffer and owned timer; install the free caret callback rather than a synthetic vtable receiver."}
// TRIED: same as 0x005C9860 - the scaffold's GraphicWin::init is stale as `void` (src/graphicwin.cpp's real recovery returns `int`, and this disassembly tests the return value), so a same-layout `GraphicWinInitShim` with the correct `int` return gets the call.
// TRIED: landed at 0.92 similarity, 410 vs 409 bytes; diverges at #45 where the trailing low-byte or/and-with-1 flag update compiles to a wider `not`/`and` sequence here, and VC6 inlines the `strlen` call as `repne scasb` where the original kept a real `call`. Tried an explicit `unsigned char` low-byte read/write for the flag update instead of a 32-bit read-modify-write; same divergence point.
// The recovered Win base is declared as a class; VC6 consequently spells its
// pointer PAVWin rather than the historical PAUWin without changing the ABI.
// symbol    ?init@EditBox@@QAEHHHHHPAVWin@@H@Z
int EditBox::init(int x, int y, int width, int height, Win *parent,
                  int focus_wrap) {
    field_B14_ = 10;
    text_[0] = 0;
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
    if (parent == 0) return 3;

    const int style_flags = (parent->iFlags_ & 0x1000) ? 0x2020 : 0x102020;
    const int result = GraphicWin::init(x, y, width, height, 0,
                                        style_flags, parent, 0, 0);
    if (result != 0) return result;

    Buffer::set_markup_enabled(0);
    Buffer::set_text_color(EditBoxTextColor1, EditBoxTextColor2,
                           EditBoxTextColor3, EditBoxTextColor4);
    Win::set_cursor(0x7f01);
    field_B40_ = 0;
    field_B44_ = strlen(text_);
    redraw();
    time_.init(edit_box_caret_callback, reinterpret_cast<int>(this), 0x1f4, 5);
    redraw();
    if (focus_wrap) field_B3C_ |= 1;
    else field_B3C_ &= ~1u;
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00615180 ?UNK1@EditBox@@QAEHPAUFont@@HHHPAUWin@@@Z 0x00615180-0x006151D6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00615180.cpp records six stack arguments and a font-height call", "GraphicWin's Buffer base begins at EditBox+0x444", "EditBox::init at 0x00614FE0 consumes the derived height"], "reason": "Initialize the real Buffer font and use its measured line height to size the edit box; retain the sixth ABI argument although the original body does not read it."}
// The historical label omits the sixth stack argument; the source symbol
// records the actual C++ ABI and the recovered Win/Font class spellings.
// symbol    ?init_with_font@EditBox@@QAEHPAVFont@@HHHPAVWin@@H@Z
int EditBox::init_with_font(Font *font, int x, int y, int width,
                            Win *parent, int) {
    if (font == 0) return 3;
    Buffer::set_font(font, 0, 0, 0);
    const int height = Buffer::text_line_height() + 2;
    return init(x, y, width, height, parent, 0);
}


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
// Deleted seed provenance: git:117282ee0186f316fd18d2d25f9ae0223c8186ae:src/unrecovered/00408010.cpp
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

/*
Purpose: Replace one edit box's text: clear the cursor; if the input exceeds
         the character limit, strip the caller's mutable input to empty,
         then append it and recompute the text length before repainting.

// ORIGINAL: 0x006151E0 ?set_text@EditBox@@QAEXPAD@Z 0x006151E0-0x0061526A
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; original identity 0x006151E0 retains name and all original spans/claims", "src/editbox.h declares the actual EditBox::set_text class member; body uses that existing owner API", "original source was FILE-mode in src/leaf_recoveries.cpp; this transfer places the unchanged C++ member definition in its actual owner TU and changes measurement mode to BODY explicitly; it does not assert historical recipe equality", "the complete source body and existing TRIED/LEVER/DEFERRED history are carried forward unchanged; only annotation mode/location and transitional SCAFFOLD classification change"], "reason": "Transfer the complete EditBox::set_text definition to its existing source owner without changing its C++ body. Keep the original FILE spans and historical claim state; BODY is the new per-member measurement scope, not a claim that the old whole-TU recipe is equivalent."}
// size      138 bytes
// prototype void (__thiscall ?set_text@EditBox@@QAEXPAD@Z)(EditBox* this, int8*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00615264
// TRIED (forwarder): reached from `EditGroup::set_text` - see the note in
//         `editbox.h`.
Status: Source-owned member body; behavior retained, exactness not measured
*/
void EditBox::set_text(char *text) {
    field_B18_ = 0;
    text_[0] = 0;
    if (text != 0) {
        if (strlen(text) != 0) {
            const unsigned int limit = field_B14_;
            if (strlen(text) > limit) {
                do {
                    text[strlen(text) - 1] = 0;
                } while (strlen(text) != 0);
            }
        }
        strcat(text_, text);
    }
    field_B40_ = 0;
    field_B44_ = static_cast<uint32_t>(strlen(text_));
    redraw();
}
