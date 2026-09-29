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
#include "stringbox.h"
#include "stringstruct.h"
#include <cstring>

namespace {

// Remove the entry currently selected by the typed StringList cursor. The
// list owns both allocations; its virtual remove hook owns any copied text.
void erase_string_list_current(StringList &list) {
    StringStructEntry *const entry = list.current_;
    StringStructEntry *const next = entry->next;
    entry->previous->next = next;
    next->previous = entry->previous;
    if (entry == list.head_) {
        list.head_ = next;
    }
    list.current_ = next;

    StringStructPayload *const payload = entry->payload;
    list.on_remove(payload);
    delete payload;
    entry->payload = 0;
    delete entry;
    --list.entry_count_;
}

}

// The getter is only an admission predicate. Reload the actual parent after
// its callback; the returned GraphicWin pointer is not the dispatch receiver.
// ORIGINAL: 0x00432720 ?on_link_clicked@StringBox@@QAEXHPAD@Z 0x00432720-0x00432761 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Full original432720..432761 returns1 on parent/getter refusal and forwards parent slot88 result otherwise; table66ADC8 selects the same override. The seed void+assembly return is an incorrect signature.", "Reload win_parent_ after virtual get_graphic_win. Its returned GraphicWin pointer is predicate-only; current parent is the callback receiver."], "reason": "Recover original GraphicWin link-hook overrides through the genuine slot88 hierarchy, preserving seed identity and historical obligations."}
// symbol    ?on_link_clicked@StringBox@@UAEHHPAD@Z
int StringBox::on_link_clicked(int id, char *link) {
    Win *parent = win_parent_;
    if (parent == 0 || parent->get_graphic_win() == 0) {
        return 1;
    }
    return static_cast<GraphicWin *>(win_parent_)->on_link_clicked(id, link);
}

// The StringBox scroll-in-progress hook is Win slot 48. Use the real
// inherited virtual member; the two values are the original event code and
// position.
// ORIGINAL: 0x00629D40 ?on_scrolling@StringBox@@QAEXHH@Z 0x00629D40-0x00629D55 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:93d09e9c95484465004f450350b6cffe1e09f40e:src/recovered/00629d40.cpp", "src/stringbox.h: StringBox derives directly from GraphicWin", "src/win.h: Win::vslot_48(int,int)"], "reason": "Forward StringBox scroll-in-progress events through inherited Win slot 48 using its two integer arguments."}
void StringBox::on_scrolling(int code, int position) {
    vslot_48(code, position);
}

// The owned StringList handles caret parsing and ID-zero insertion; a
// successful insertion asks the box to re-fit its visible rows.
// ORIGINAL: 0x00629750 ?add_special@StringBox@@QAEHPAD@Z 0x00629750-0x00629774
// RECOVERY_BINDING: v1 {"evidence": ["git:93d09e9c95484465004f450350b6cffe1e09f40e:src/recovered/units/00629750.cpp", "src/stringlist.cpp StringList::add_special; src/stringstruct.h", "src/stringbox.h"], "reason": "Delegate caret-prefix parsing and ID-zero insertion to the owned StringList, then request row fitting after a successful insert."}
int StringBox::add_special(char *text) {
    if (stringList_.add_special(text) == 0) {
        add_fixup();
    }
    return 0;
}

// Close the real GraphicWin base, clear the owned StringList, and reset its
// cursor before closing the embedded Scroll. The list's typed removal path
// dispatches StringList::on_remove and destroys both node types.
// ORIGINAL: 0x00629210 ?close@StringBox@@QAEXXZ 0x00629210-0x006292C9
// RECOVERY_BINDING: v1 {"evidence": ["git:93d09e9c95484465004f450350b6cffe1e09f40e:src/unrecovered/00629210.cpp", "src/stringbox.h", "src/stringstruct.h", "src/scroll.h"], "reason": "Close the GraphicWin base, release owned StringList entries through their typed virtual removal hook, reset the list cursor, and close the embedded Scroll."}
void StringBox::close() {
    visible_rows_ = 0xffffffff;
    top_line_ = 0;
    list_override_ = 0;
    per_line_draw_ = 0;
    GraphicWin::close();
    stringList_.remove_all();
    stringList_.current_position_ = 0;
    scroll_.close();
}

// Reinitialization closes the old window/list/scroll before installing a new
// GraphicWin surface. An optional caller list is borrowed, not copied.
// ORIGINAL: 0x006292D0 ?init@StringBox@@QAEXHHHHPAUWin@@PAUStringList@@H@Z 0x006292D0-0x00629486
// RECOVERY_BINDING: v1 {"evidence": ["original:0x006292D0-0x00629486;seed:src/recovered/units/006292d0.cpp;source:src/stringbox.h;source:src/stringstruct.h;source:src/scroll.h"], "reason": "Reinitialize GraphicWin, StringList and embedded Scroll through their real owned subobjects."}
// symbol    ?init@StringBox@@QAEXHHHHPAVWin@@PAVStringList@@H@Z
void StringBox::init(int x, int y, int width, int height, Win *parent,
                     StringList *list, int flags) {
    close();
    if (flags == -1) {
        flags = 0x1000120;
    }
    if (GraphicWin::init(x, y, width, height, 0, flags, parent, 0, 0) != 0) {
        return;
    }
    if (list != 0) {
        list_override_ = list;
    } else {
        stringList_.remove_all();
    }
    stringList_.current_position_ = 0;
    stringList_.allocator_ = 0;
    scroll_.init_vert(height - ScrollDefaultThickness, 0,
                      -dib_.bmiHeader.biHeight, this, 0);
    scroll_.hide();
    scroll_.range_minimum_ = 0;
    scroll_.range_maximum_ = 0;
}

// Refit the selected list to the visible row budget and keep the embedded
// scrollbar's range, paging and cursor synchronized with the measured text.
// ORIGINAL: 0x00629490 ?add_fixup@StringBox@@QAEXXZ 0x00629490-0x00629703
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00629490-0x00629703;seed:src/recovered/units/00629490.cpp;source:src/stringbox.h;source:src/stringstruct.h;source:src/scroll.h"], "reason": "Fit the selected StringList to the visible text area and synchronize the embedded Scroll state."}
void StringBox::add_fixup() {
    int line_height = text_line_height();
    if (line_height == 0) {
        line_height = 12;
    }
    StringList &list = list_override_ != 0 ? *list_override_ : stringList_;
    const int width = dib_.bmiHeader.biWidth;
    const int text_width = (scroll_.iSomeFlag_ & WinFlagVisible) != 0
                               ? width - scroll_.bar_thickness_ : width;

    if (static_cast<int>(visible_rows_) > 0) {
        int measured = write_strings_height(&list, text_width, 0);
        while (measured / line_height > static_cast<int>(visible_rows_)) {
            if (list.entry_count_ > 0) {
                list.current_ = list.head_;
                list.current_position_ = 0;
            }
            if (list.head_ != 0) {
                erase_string_list_current(list);
                if (list.entry_count_ == 0) {
                    list.head_ = 0;
                }
                if (list.current_position_ > list.entry_count_ - 1) {
                    list.current_position_ = list.entry_count_ - 1;
                }
            }
            measured = write_strings_height(&list, text_width, 0);
        }
    }

    scroll_.range_maximum_ = list.entry_count_;
    scroll_.paging_ = -1 - (dib_.bmiHeader.biHeight / line_height);
    if ((scroll_.iSomeFlag_ & WinFlagVisible) == 0) {
        if (write_strings_height(&list, width, 0) > -dib_.bmiHeader.biHeight) {
            scroll_.show(0);
        } else {
            scroll_.hide();
        }
    }

    if (is_visible() != 0) {
        if ((scroll_.iSomeFlag_ & WinFlagVisible) != 0) {
            if (list.entry_count_ > 0) {
                list.current_ = list.head_;
                list.current_position_ = 0;
            }
            if (-(text_line_height() + dib_.bmiHeader.biHeight) >= 0) {
                int measured = 0;
                do {
                    if (list.head_ != 0) {
                        list.current_ = list.current_->previous;
                        if (--list.current_position_ == -1) {
                            list.current_position_ = list.entry_count_ - 1;
                        }
                    }
                    char *text = list.current_ != 0
                                     ? list.current_entry()->text : 0;
                    measured += wrap_height(text, width);
                } while (measured <= -(text_line_height() + dib_.bmiHeader.biHeight));
            }
            list.next_entry();
            scroll_.set_pos(list.current_position_);
            top_line_ = scroll_.position_;
        }
        redraw();
    }
}

// This is the canonical product body for original 0x00629780. Clearing may
// target a borrowed alternate list; destruction of that list remains its
// owner's responsibility. The typed removal path invokes the virtual
// text-release hook instead of treating the list as raw offsets.
// ORIGINAL: 0x00629780 ?clear@StringBox@@QAEXXZ 0x00629780-0x006298A4
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00629780.cpp; image:0x00629780-0x006298A4; source:src/stringbox.h:StringBox; source:src/leaf_recoveries.cpp:sub_441100"], "reason": "Replace the old link-only shim with the real StringBox method: original code selects the borrowed or embedded StringList, destroys its entries, hides the Scroll, zeros the top line, and redraws if visible. Typed ownership expresses those effects without raw self offsets."}
void StringBox::clear() {
    StringList &list = list_override_ != 0 ? *list_override_ : stringList_;
    list.remove_all();
    scroll_.hide();
    top_line_ = 0;
    if (is_visible() != 0) {
        redraw();
    }
}

// ORIGINAL: 0x00629B70 ?on_redraw@StringBox@@QAEXXZ 0x00629B70-0x00629D1C
// RECOVERY_BINDING: v1 {"evidence": ["original:0x00629B70-0x00629D1C;seed:src/recovered/units/00629b70.cpp;source:src/stringbox.h;source:src/stringstruct.h;source:src/scroll.h"], "reason": "Render the selected StringList through inherited Buffer and GraphicWin drawing methods."}
// symbol    ?on_redraw@StringBox@@UAEXXZ
void StringBox::on_redraw() {
    clear_links();
    StringList &list = list_override_ != 0 ? *list_override_ : stringList_;
    fill(9);
    int text_width = dib_.bmiHeader.biWidth;
    if ((scroll_.iSomeFlag_ & WinFlagVisible) != 0) {
        text_width -= scroll_.bar_thickness_;
    }

    int position = static_cast<int>(top_line_);
    if (position <= list.entry_count_ - 1) {
        list.current_ = list.head_;
        if (position < 0) {
            int steps = abs(position);
            if (steps <= list.entry_count_) {
                while (steps-- > 0) {
                    list.current_ = list.current_->previous;
                }
                position += list.entry_count_;
                list.current_position_ = position;
            }
        } else {
            int steps = position;
            while (steps-- > 0) {
                list.current_ = list.current_->next;
            }
            list.current_position_ = position;
        }
    }

    if (per_line_draw_ == 0) {
        write_strings(&list, 0, 0, text_width, 2);
        return;
    }
    int remaining = -dib_.bmiHeader.biHeight;
    for (int index = 0; index < list.entry_count_ && remaining > 0; ++index) {
        StringStructPayload *payload = list.head_ != 0
                                           ? list.current_->payload : 0;
        char *text = payload != 0 ? payload->text : 0;
        remaining -= wrap_height(text, text_width);
        if (remaining < 0) {
            break;
        }
        if (payload == 0 || (payload->field_8_ & 2) == 0) {
            wrap(text, 0, remaining, text_width);
        } else {
            wrap_cent(text, 0, remaining, text_width);
        }
        if (list.head_ != 0) {
            list.current_ = list.current_->next;
            if (++list.current_position_ == list.entry_count_) {
                list.current_position_ = 0;
            }
        }
    }
}

// The member and base destructors follow this body implicitly, in reverse
// construction order: StringList, Scroll, then GraphicWin.
// ORIGINAL: 0x004325C0 ??1StringBox@@QAE@XZ 0x004325C0-0x00432713;0x006533B0-0x0065341D
// RECOVERY_BINDING: v1 {"evidence": ["original:0x004325C0-0x00432713;0x006533B0-0x0065341D;seed:src/recovered/units/004325c0.cpp;source:src/stringbox.h;source:src/stringstruct.h;source:src/scroll.h"], "reason": "Run StringBox cleanup then let C++ destroy its StringList, Scroll and GraphicWin subobjects in lifetime order."}
// symbol    ??1StringBox@@UAE@XZ
StringBox::~StringBox() {
    close();
}

// Remove the first list entry with this ID, retaining the original cursor
// position side effect and then re-fitting the visible rows.
// ORIGINAL: 0x006298B0 ?UNK1@StringBox@@QAEXH@Z 0x006298B0-0x0062997B
// RECOVERY_BINDING: v1 {"evidence": ["git:93d09e9c95484465004f450350b6cffe1e09f40e:src/unrecovered/006298b0.cpp", "src/stringbox.h", "src/stringstruct.h", "src/recovered/units/00629490.cpp"], "reason": "Remove the first StringList entry with the requested ID, preserve its cursor effects, and re-fit visible rows."}
void StringBox::UNK1(int id) {
    StringList &list = stringList_;
    if (list.head_ != 0) {
        const int count = list.entry_count_;
        int visited = 0;
        bool found = false;
        if (count > 0) {
            do {
                if (list.current_->id == id) {
                    found = true;
                    break;
                }
                list.current_ = list.current_->next;
                ++visited;
            } while (visited < count);
        }
        if (found) {
            erase_string_list_current(list);
        }
        if (list.entry_count_ == 0) {
            list.head_ = 0;
        }
        list.current_position_ = list.entry_count_ - 1;
    }
    add_fixup();
}

// Preserve the seed's inverted/wrapping predicate exactly: retain an ID only
// when ID < first_bound && ID > second_bound. Remove all others while the
// cursor and changing count follow the original list walk.
// ORIGINAL: 0x00629980 ?UNK2@StringBox@@QAEXHH@Z 0x00629980-0x00629A63 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:93d09e9c95484465004f450350b6cffe1e09f40e:src/unrecovered/00629980.cpp", "src/stringbox.h", "src/stringstruct.h", "src/recovered/units/00629490.cpp"], "reason": "Retain IDs in the original descending-bound interval and remove other entries, preserving the changing-count list walk."}
void StringBox::UNK2(int first_bound, int second_bound) {
    StringList &list = stringList_;
    if (list.head_ != 0) {
        int visited = 0;
        if (list.entry_count_ > 0) {
            do {
                const uint32_t id = static_cast<uint32_t>(list.current_->id);
                if (id < static_cast<uint32_t>(first_bound) &&
                    id > static_cast<uint32_t>(second_bound)) {
                    list.current_ = list.current_->next;
                } else {
                    erase_string_list_current(list);
                }
                ++visited;
            } while (visited < list.entry_count_);
        }
        if (list.entry_count_ == 0) {
            list.head_ = 0;
        }
        list.current_position_ = list.entry_count_ - 1;
    }
    add_fixup();
}

// Drop IDs at or below count; higher IDs are rebased by count. Only a
// removal changes the scrollbar fit, matching the original operation.
// ORIGINAL: 0x00629A70 ?clip_ids@StringBox@@QAEXK@Z 0x00629A70-0x00629B69
// RECOVERY_BINDING: v1 {"evidence": ["git:93d09e9c95484465004f450350b6cffe1e09f40e:src/unrecovered/00629a70.cpp", "src/stringbox.h", "src/stringstruct.h", "src/recovered/units/00629490.cpp"], "reason": "Remove list IDs at or below the clipping count and rebase larger IDs while preserving list cursor and fit behavior."}
void StringBox::clip_ids(unsigned long count) {
    StringList &list = stringList_;
    if (list.head_ != 0) {
        bool removed = false;
        int visited = 0;
        if (list.entry_count_ > 0) {
            do {
                const uint32_t id = static_cast<uint32_t>(list.current_->id);
                if (id <= count) {
                    removed = true;
                    erase_string_list_current(list);
                } else {
                    list.current_->id = static_cast<int>(id - count);
                    list.current_ = list.current_->next;
                }
                ++visited;
            } while (visited < list.entry_count_);
        }
        if (list.entry_count_ == 0) {
            list.head_ = 0;
        }
        list.current_position_ = list.entry_count_ - 1;
        if (removed) {
            add_fixup();
        }
    }
}

/*
Purpose: Construct a StringBox: run the GraphicWin base construction, build
         the Scroll at 0xA20 and the StringList at 0x2B70, install this
         class's own two virtual tables, and clear the window state.
// ORIGINAL: 0x00629110 ??0StringBox@@QAE@XZ 0x00629110-0x00629208;0x00663300-0x00663312 BYTE_EXACT
// LEVER: THE StringList IS A REAL, VIRTUALLY-DERIVED MEMBER. The image's
//        frame reserves TWO dwords below its EH record (`sub esp, 8`) and
//        stores `this` into one and zero into the other before the base call;
//        every spelling with the StringList held by layout reserved one. The
//        two-slot frame is what VC6 emits only when the constructed hierarchy
//        really derives virtually - measured against cl 12.00.8168 in
//        isolation (2026-08-29), and landed here by making StringStruct
//        `: public virtual StringAllocationBase` (stringstruct.h) with the
//        owner capture in StringAllocationBase's inline ctor and the field
//        zeroing in StringStruct's. The whole 0x2B70 block - vbtable, vbase
//        vtable, owner capture, both vtable stages, the five zeroes - is then
//        COMPILER-GENERATED, inlined in the image's exact store order, with
//        no call anywhere: the earlier hand-written body reproduced the block
//        but never the frame.
// LEVER: DERIVE FROM GraphicWin DIRECTLY, not through ConstructedGraphicWin.
//        The shim is an empty pass-through now that GraphicWin has a real
//        constructor, but its inlined implicit constructor stores the SHIM's
//        own two vftables between the base call and the Scroll member - a
//        pair the image does not have. Dropping the layer removed exactly
//        those two instructions; the funclet (cold span) is the image's own
//        `mov ecx, [this-slot] / jmp ??1StringBox` shape because ~StringBox
//        stays declared in the header and defined only in pending_bodies.cpp.
// TRIED: BYTE_EXACT with the StringList held by layout, and the wall was the
//        frame, not the block: the hand-written staging measured 46 of 47
//        instructions with only `sub esp, 8` against `push ecx` disagreeing.
// TRIED: a `static void ...(char *self, unsigned offset)` helper for the
//        StringList staging - prodpicker.cpp's shape. VC6 declines to inline
//        it and emits `push 0x2b70 / push esi / call`: 0.683 similar.
// size      266 bytes
// prototype void (__thiscall ??0StringBox@@QAE@XZ)(StringBox* this)
// callers   6   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4CF0 0x006051D0
Return Value: Instance pointer in EAX
Status: Complete
*/
StringBox::StringBox() {
    top_line_ = 0;
    list_override_ = 0;
    per_line_draw_ = 0;
    visible_rows_ = -1;
}

/*
Purpose: Add a string to the box - stage the text, index and a cleared flag
         into the string struct at 0x2B70, add it, and run the fixup pass when
         the struct reports it did not take.
// ORIGINAL: 0x00629710 ?add@StringBox@@QAEXPADHH@Z 0x00629710-0x00629749
// size      57 bytes
// prototype void (__thiscall ?add@StringBox@@QAEXPADHH@Z)(StringBox* this, int8*, int, int)
// callers   6   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100 0x00629490
Return Value: n/a
Status: Complete
*/
void StringBox::add(char *text, int index, int flag) {
    // ONE ADDRESS, TAKEN ONCE. The image computes `ecx = &field_2B70_` a
    // single time (`lea ecx, [esi + 0x2b70]`) and reaches every field below
    // it - the StringList's field_1C_, field_20_ and field_24_ and the
    // StringStruct::add call itself - off THAT register. Storing through
    // named members instead makes VC6 re-derive each one from `this`.
    char *const base = reinterpret_cast<char *>(&stringList_);
    std::memcpy(base + 0x1c, &text, sizeof(text));
    *reinterpret_cast<uint32_t *>(base + 0x20) = static_cast<uint32_t>(flag);
    *reinterpret_cast<uint32_t *>(base + 0x24) = 0;
    // CALLED BY NAME, same as Dialog::item at 0x00609990: a real
    // `StringStruct::add` reaches the image's `call rel32`, where the
    // function-pointer seam this file used to bind compiles
    // `call dword ptr [0]`.
    if (reinterpret_cast<StringStruct *>(base)->add(index) == 0) {
        StringBox::add_fixup();
    }
    // TRIED: the image emits one further `xor eax, eax` right before the
    // epilogue, on the fixup-taken path only - MISMATCH stays at 14/18
    // (0.971 similar) without it. Tried: an early-return guard clause
    // instead of the if-block (`if (result != 0) return;`); dropping the
    // `StringBox::` qualifier on the call. Neither changes the codegen, and
    // `add` is genuinely void (`...QAEX...`), so there is no return value
    // to re-materialise. Plateaued here.
}


/*
Purpose: Record the scroll position and repaint through virtual slot 62.
// ORIGINAL: 0x00629D20 ?on_scrolled@StringBox@@QAEXHH@Z 0x00629D20-0x00629D35 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?on_scrolled@StringBox@@QAEXHH@Z)(StringBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00629D2C
Return Value: n/a
Status: Complete
*/
void StringBox::on_scrolled(int code, int pos) {
    top_line_ = pos;
    redraw();
}

/*
Purpose: Step the receiver back to the subobject ??_GStringBox@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00432830 ??_GStringBox@@WEEE@AEPAXI@Z 0x00432830-0x0043283B BYTE_EXACT
// symbol    ??_EStringBox@@WEEE@AEPAXI@Z
// CORRECTED from ??3StringBox@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00432770` into
//   ??_GStringBox@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
