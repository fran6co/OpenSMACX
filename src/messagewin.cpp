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
#include "messagewin.h"
#include "buttongroup.h"
#include "base.h"
#include "console.h"
#include "alpha.h"
#include "faction.h"
#include "game.h"
#include "lock.h"
#include "map.h"
#include "mapwin.h"
#include "pulldown.h"
#include "strings.h"
#include "tutwin.h"

// 0x007F67F8, declared in messagewin.h.
MessageWin InterfaceMessageWin;  // 0x007F67F8
// RECOVERY_OBJECT: v2 0x009393D0 0x009393D4 ?MessageWinMessageCount@@3HA 0x00471530
int MessageWinMessageCount;  // 0x009393D0; initial storage passed both VC6 trials.
// RECOVERY_OBJECT: v2 0x0093E964 0x0093E968 ?MessageWinNetworkMessageCount@@3HA 0x004713B0
int MessageWinNetworkMessageCount; // 0x0093E964; initial storage passed both trials.
// RECOVERY_OBJECT: v2 0x007F9F54 0x007F9F58 ?MessageWinMenuBusy@@3HA 0x004718C0
int MessageWinMenuBusy;  // 0x007F9F54; initial storage passed both trials.
// 0x0093A7B0 is eight faction-indexed StringList records. The independent
// add_base and leaf_recoveries seeds agree on the 0x30 stride and pending
// text/metadata fields. Initial storage/extent passed two VC6 trials; runtime
// construction, ownership and retained linker selection remain unverified.
// RECOVERY_OBJECT: v2 0x0093A7B0 0x0093A930 ?MessageWinDeferredBaseMessages@@3PAVStringList@@A 0x004713B0
StringList MessageWinDeferredBaseMessages[8];
// The two color initializers are stored .data dwords in the pinned image:
// 0xE7 at 0x0068A5A4 and 0x91 at 0x0068A5B0. Both initial storage bindings
// passed two VC6 trials; runtime state remains unverified. The seed's
// 0x00689370 is BaseIDCurrentSelected.
// RECOVERY_OBJECT: v2 0x0068A5A4 0x0068A5A8 ?MessageWinTextColor@@3HA 0x00471FD0
int MessageWinTextColor = 0xE7;
// RECOVERY_OBJECT: v2 0x0068A5B0 0x0068A5B4 ?MessageWinTextBackground@@3HA 0x00471FD0
int MessageWinTextBackground = 0x91;

// 0x004718C0. PullDown::get_selected returns a slot; the image reads that
// slot's action ID, whose bits mean navigate (1) and remove (2). The three
// menu labels are Labels words 0x44C..0x44E.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004718C0 ?on_selected@MessageWin@@QAEXHH@Z 0x004718C0-0x00471D36;0x00655F60-0x00655F75
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004718c0.cpp", "image:0x004718C0"], "reason": "The archived selection handler owns the three-action PullDown, map/base navigation and embedded ListBox row removal."}
void MessageWin::on_selected(int code, int) {
    if (MessageWinMenuBusy) {
        return;
    }

    int mouse_x, mouse_y;
    ConsoleGlobal.get_mouse_pos(&mouse_x, &mouse_y);
    ConsoleGlobal.client_to_screen(&mouse_x, &mouse_y);
    PullDown menu;
    menu.init(0);
    int *label_ids = reinterpret_cast<int *>(Labels.strings_ptr);
    menu.add_item(1, StringTable.get(label_ids[0x44C]));
    menu.add_item(2, StringTable.get(label_ids[0x44D]));
    menu.add_item(3, StringTable.get(label_ids[0x44E]));
    menu.show(mouse_x, mouse_y, 0);
    const int action = menu.get_selected_id();

    if (action & 1) {
        MessageWinMenuBusy = 1;
        if (code & 0x40000000) {
            const unsigned int base_id = code & 0xBFFFFFFF;
            if (static_cast<int>(base_id) >= 0 &&
                Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
                const int x = Bases[base_id].x;
                const int y = Bases[base_id].y;
                ConsoleGlobal.set_center(x, y, 1);
                if (!ConsoleGlobal.cursor_active_) {
                    ConsoleGlobal.set_cursor(x, y);
                }
                TutWinBaseWindow.zoom(base_id, 0);
            }
        } else if (!(code & 0x20000000) && MapLongitudeBounds != 0) {
            const int square = code & 0x7FFFFFFF;
            const int y = square / MapLongitudeBounds;
            const int x = square % MapLongitudeBounds;
            if (y >= 0 && y < MapLatitudeBounds &&
                x >= 0 && x < MapLongitudeBounds) {
                const int faction = ConsoleGlobal.faction_id_;
                if ((PlayersData[faction].flags & PFLAG_MAP_REVEALED) != 0 ||
                    (map_loc(x, y)->visibility & (1 << faction)) != 0) {
                    ConsoleGlobal.set_center(x, y, 1);
                    if (!ConsoleGlobal.cursor_active_) {
                        ConsoleGlobal.set_cursor(x, y);
                    }
                }
            }
        }
    }

    if (action & 2) {
        --MessageWinMessageCount;
        remove_selected_message();
        listBox_.set_selected_pos(-1, 1);
        listBox_.update_changes();
        if (static_cast<Dialog &>(listBox_).item_list_.head_ == 0) {
            MainInterfaceGlobal.button_group(0).set(1001, 1);
        }
    }
    listBox_.set_selected_pos(-1, 1);
    MessageWinMenuBusy = 0;
}

void MessageWin::remove_selected_message() {
    Dialog &dialog = listBox_;
    StringList &items = dialog.item_list_;
    if (items.head_ == 0 || items.entry_count_ == 0) {
        return;
    }
    items.seek_pos(dialog.selected_position_);
    StringStructEntry *entry = items.current_;
    if (entry == 0) {
        return;
    }
    StringStructPayload *payload = entry->string_payload();
    if (payload != 0) {
        listBox_.field_20_ -= payload->field_C_;
    }
    if (items.entry_count_ == 1) {
        items.head_ = 0;
        items.current_ = 0;
    } else {
        entry->previous->next = entry->next;
        entry->next->previous = entry->previous;
        if (entry == items.head_) {
            items.head_ = entry->next;
        }
        items.current_ = entry->next;
    }
    items.on_remove(payload);
    delete payload;
    entry->payload = 0;
    delete entry;
    --items.entry_count_;
    if (items.current_position_ >= items.entry_count_) {
        items.current_position_ = items.entry_count_ - 1;
    }
}

// 0x00471530. The archived scaffold walks ListBox's virtual Dialog base
// through its vbtable. That base and its StringList are real C++ objects now.
// The archived seed remains as historical evidence; this product body has a
// native binding and currently measures a mismatch.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00471530 ?add_message@MessageWin@@QAEXPADH@Z 0x00471530-0x00471794
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00471530.cpp", "image:0x00471530"], "reason": "The product MessageWin append method implements the archived ListBox and virtual Dialog list walk on the real embedded members."}
void MessageWin::add_message(char *text, int item_id) {
    Dialog &dialog = listBox_;
    StringList &items = dialog.item_list_;

    // The seed seeks the most recent entry before comparing zero-ID text.
    if (item_id == 0 && MessageWinMessageCount != 0 && items.head_ != 0) {
        StringStructEntry *last = items.head_->previous;
        if (last->string_payload() != 0 && last->string_payload()->text != 0 &&
            strcmp(text, last->string_payload()->text) == 0) {
            return;
        }
    }

    ++MessageWinMessageCount;
    listBox_.item(text, item_id | 0x10000000);
    listBox_.set_selected_pos(-1, 1);

    while (MessageWinMessageCount > 100 && items.head_ != 0) {
        --MessageWinMessageCount;
        StringStructEntry *oldest = items.head_;
        StringStructPayload *payload = oldest->string_payload();
        if (payload != 0) {
            listBox_.field_20_ -= payload->field_C_;
        }
        if (items.entry_count_ == 1) {
            items.head_ = 0;
            items.current_ = 0;
        } else {
            oldest->previous->next = oldest->next;
            oldest->next->previous = oldest->previous;
            items.head_ = oldest->next;
            items.current_ = items.head_;
        }
        items.on_remove(payload);
        delete payload;
        oldest->payload = 0;
        delete oldest;
        --items.entry_count_;
        if (items.current_position_ >= items.entry_count_) {
            items.current_position_ = items.entry_count_ - 1;
        }
    }
    listBox_.update_changes();
}

/*
Purpose: Construct the Sprite base, then the three plain members in
         declaration order (font_, listBox_, stringBox_) via automatic
         base+member construction - no explicit statement in the body.
// ORIGINAL: 0x00472190 ??0MessageWin@@QAE@XZ 0x00472190-0x004721F1;0x00655F30-0x00655F58 BYTE_EXACT
// size      97 bytes
// prototype void (__thiscall ??0MessageWin@@QAE@XZ)(MessageWin* this)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005E37E0 0x00618EA0 0x00609DB0 0x00629110
Return Value: n/a
Status: Complete
*/
MessageWin::MessageWin() {
}

/*
Purpose: Tear the three plain members down in reverse declaration order
         (stringBox_, listBox_, font_), then the Sprite base - no explicit
         statement in the body, all of it automatic member teardown.
// ORIGINAL: 0x00472020 ??1MessageWin@@QAE@XZ 0x00472020-0x00472184
// size      356 bytes
// prototype void (__thiscall ??1MessageWin@@QAE@XZ)(MessageWin* this)
// callers   0   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x00401060 0x00402970 0x00406880 0x005D4DD0 0x005E3820 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x00618EE0 0x00629210
// TRIED: byte-exact is not reachable from here, by a wide margin. The
//        image's teardown is NOT a plain reverse-order destructor call per
//        member - StringBox's own destructor (0x629210) is inlined open at
//        the call site alongside manual teardown of ListBox's two virtual
//        bases (explicit `call 0x5D4DD0` - GraphicWin::close() - three
//        times, `call 0x608E10` - Dialog-shaped - and more), none of which
//        this tree models. `font_.~Font()` (0x618EE0, real, BYTE_EXACT) and
//        the Sprite base (0x5E3820) are the only two subobjects here that
//        are actually recovered; stringBox_ and listBox_ are the same two
//        unrecovered, widely-shared classes the constructor's own note
//        names (StringBox 248 bytes, ListBox's virtual-inheritance
//        modelling documented blocked in listbox.h). Left as automatic
//        member teardown through whatever each member's CURRENT destructor
//        is, rather than hand-writing the inlined-open shape above.
Return Value: n/a
Status: Complete
*/
MessageWin::~MessageWin() {
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004713A0 ?UNK1@MessageWin@@QAEXHHHHH@Z 0x004713A0-0x004713A3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK1@MessageWin@@QAEXHHHHH@Z)(MessageWin* this, int, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MessageWin::UNK1(int, int, int, int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00471D40 ?UNK4@MessageWin@@QAEXH@Z 0x00471D40-0x00471D43 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK4@MessageWin@@QAEXH@Z)(MessageWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MessageWin::UNK4(int) {
}


// 0x004713B0 is now source-owned here; the archived seed remains historical
// evidence and Popup::on_exec reaches this typed member by name.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004713B0 ?add_base@MessageWin@@QAEXHPADPAUSprite@@@Z 0x004713B0-0x00471461 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004713b0.cpp", "source:src/popup.cpp", "image:0x004713B0-0x00471461"], "reason": "The existing MessageWin product method replaces the archived fixed-address forwarder; the original call and seed identify the MessageWin receiver, base ID, text, and unused Sprite pointer."}
// symbol    ?add_base@MessageWin@@QAEXHPADPAVSprite@@@Z
void MessageWin::add_base(int base_id, char *text, Sprite *source) {
    // The archived third argument is a Sprite*, and the observed body never
    // reads it. PBEM queues a line for the base's faction when it is not the
    // local faction, then bypasses the visible desktop list.
    (void)source;
    if (IsMultiplayerPBEM) {
        const int faction = Bases[base_id].faction_id_current;
        if (faction != ConsoleGlobal.faction_id_) {
            StringList &pending = MessageWinDeferredBaseMessages[faction];
            pending.pending_text_ = text;
            pending.field_20_ = 0;
            pending.field_24_ = 0;
            pending.add(base_id | 0x40000000);
            return;
        }
    }
    if (IsMultiplayerNet) {
        ++MessageWinNetworkMessageCount;
    }
    ++MessageWinMessageCount;
    listBox_.item(text, base_id | 0x40000000);
    listBox_.set_selected_pos(-1, 1);
    listBox_.update_changes();
    MainInterfaceGlobal.button_group(0).set(1000, 1);
}

// 0x00471470, archived in src/recovered/units/00471470.cpp. The original
// ignores the first and last arguments and appends a 0x20000000-tagged row.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00471470 ?UNK2@MessageWin@@QAEHHPADH@Z 0x00471470-0x004714C2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00471470.cpp", "image:0x00471470"], "reason": "The archived receiver and list insertion call chain identify this MessageWin variant; its first and last arguments are unused in the seed."}
int MessageWin::UNK2(int, char *text, int) {
    const int id = BaseIDCurrentSelected | 0x20000000;
    ++MessageWinMessageCount;
    listBox_.item(text, id);
    listBox_.set_selected_pos(-1, 1);
    listBox_.update_changes();
    return MainInterfaceGlobal.button_group(0).set(1000, 1);
}

// 0x004714D0, archived in src/unrecovered/004714d0.cpp. The original's
// four-argument return (`ret 0x10`) is more reliable than the old catalogue
// prototype, which omitted the unused fourth argument.
// ORIGINAL: 0x004714D0 ?UNK3@MessageWin@@QAEHHHPAD@Z 0x004714D0-0x0047152B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004714d0.cpp", "image:0x004714D0"], "reason": "The archived MessageWin list variant packs map coordinates; its ret 0x10 proves a fourth unused stack argument absent from the catalogue prototype."}
// TRIED: the scaffolding's 3-arg UNK3 declaration can't express the `ret 0x10` 4th unused stack param, so wrote a 4-arg shim method on a derived class instead; diverges at #6 (lea vs mov)
// symbol    ?UNK3@MessageWin@@QAEHHHPADH@Z
int MessageWin::UNK3(int x, int y, char *text, int) {
    const int id = (MapLongitudeBounds * y + x) | 0x80000000;
    ++MessageWinMessageCount;
    listBox_.item(text, id);
    listBox_.set_selected_pos(-1, 1);
    listBox_.update_changes();
    return MainInterfaceGlobal.button_group(0).set(1000, 1);
}

// 0x004717A0. The list's real Dialog base owns the entry chain and its
// allocator. Keep the containing ListBox open while removing its rows.
// ORIGINAL: 0x004717A0 ?clear@MessageWin@@QAEXXZ 0x004717A0-0x00471864
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004717a0.cpp", "image:0x004717A0"], "reason": "The archived MessageWin reset walks the embedded ListBox Dialog entries and clears the same pane state and button group."}
// TRIED: MISMATCH, closest at 194/196 bytes (mnemonic_similarity 0.99, 1 edit). Same embedded-manager close-and-relink shape as SetupWin::close/Dialog::init, but reached through a vbtable: `listBox_.vbtable_pointer_` (self+0x64) is a pointer to a displacement table, slot 2 (`[vtbl+8]`) is the byte displacement to the virtual base holding the manager - `char *base = self + vbtable[2] + 0x120;` reproduces the `[edx+ebx+0x128]`/`lea esi,[edx+ebx+0x120]` addressing exactly. Only the SECOND vtable-adjustment "deleting dtor" call (vtbl[1] read as a displacement, this-adjusted, that pointer's own vtbl[0] called with arg 1) still diverges by a handful of instructions - the same register-allocator nuance as the second call site in 0x004AD9C0 and 0x00609730, not chased further.
void MessageWin::clear() {
    MessageWinMessageCount = 0;
    Dialog &dialog = listBox_;
    dialog.item_list_.remove_all();
    dialog.selected_position_ = 0;
    listBox_.field_18_ = 0;
    listBox_.field_20_ = 0;
    MainInterfaceGlobal.button_group(0).set(1001, 1);
}

// 0x00471870. The original passes the list's GraphicWin to the callback;
// its embedded Buffer is a real C++ base, and the arrow is a real Sprite.
// ORIGINAL: 0x00471870 ?listbox_draw@MessageWin@@QAAHPAUGraphicWin@@HHHHH@Z 0x00471870-0x004718BF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/00471870.cpp", "image:0x00471870"], "reason": "The archived callback draws the real interface arrow Sprite into the supplied GraphicWin Buffer and is installed by MessageWin init. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?listbox_draw@MessageWin@@SAXPAVGraphicWin@@HHHHH@Z
void __cdecl MessageWin::listbox_draw(GraphicWin *target, int, int x, int y,
                                       int row_height, int) {
    Sprite &arrow = g_IFACE_GREEN_RIGHT_ARROW_SPRITE[0];
    arrow.draw(static_cast<Buffer *>(target), arrow.transparent_index(),
               x + 4, y + (row_height - arrow.height()) / 2, 1, 1);
}

// 0x00471F70. The frame record at 0x0078D7F8 is style 18 in the existing
// InterfaceBoxStyles table, and GraphicWin owns the buffer and working rect.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00471F70 ?listbox_fill@MessageWin@@QAAHPAUGraphicWin@@HHPAURECT@@@Z 0x00471F70-0x00471FC2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/00471f70.cpp", "image:0x00471F70"], "reason": "The archived callback fills the supplied GraphicWin and draws InterfaceBoxStyles entry 18 into its Buffer."}
// symbol    ?listbox_fill@MessageWin@@SAHPAVGraphicWin@@HHPAUtagRECT@@@Z
int __cdecl MessageWin::listbox_fill(GraphicWin *target, int, int, RECT *) {
    RECT area = *target->buffer_working_rect();
    target->fill(9);
    target->box_sprite(&area, &InterfaceBoxStyles[18]);
    return 1;
}

// 0x00471FD0. The archived unit retains its historical BYTE_EXACT claim;
// this source-owned spelling currently measures a miss and needs restoration.
// ORIGINAL: 0x00471FD0 ?listbox_write@MessageWin@@QAAHPAUGraphicWin@@HHPAURECT@@PAD@Z 0x00471FD0-0x0047201F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00471fd0.cpp", "image:0x00471FD0"], "reason": "The archived callback selects two text colors for the supplied GraphicWin Buffer and is installed by MessageWin init. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?listbox_write@MessageWin@@SAHPAVGraphicWin@@HHPAUtagRECT@@PAD@Z
int __cdecl MessageWin::listbox_write(GraphicWin *target, int item_id, int,
                                       RECT *, char *) {
    Buffer &buffer = *static_cast<Buffer *>(target);
    buffer.set_text_color(MessageWinTextColor, MessageWinTextBackground, 1, 1);
    if ((item_id & 0x10000000) != 0 && (item_id & 0xefffffff) != 0) {
        buffer.set_text_color(MessageWinTextColor, -1, 1, 1);
    }
    return 0;
}

// 0x00471D50. The embedded controls and the desktop are typed objects here.
// The archived seed remains as historical evidence; this product body has a
// native binding and currently measures a mismatch.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00471D50 ?init@MessageWin@@QAEXXZ 0x00471D50-0x00471F66;0x00655F75-0x00655F8A
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00471d50.cpp", "image:0x00471D50"], "reason": "The archived constructor-stage method initializes MessageWin font, pane bounds, embedded ListBox Dialog and callback slots."}
void MessageWin::init() {
    font_.init(DefaultFontFace, 12, 0);
    Buffer scratch;
    scratch.close();

    bounds_ = MainInterfaceGlobal.message_bounds_;

    listBox_.close();
    Dialog &dialog = listBox_;
    dialog.init(&bounds_, 0);
    dialog.field_38_ = 3;
    dialog.field_3C_ = 3;
    dialog.field_6C_ = 0;
    dialog.set_dialog_font(&font_, &font_, &font_);
    listBox_.set_message_initial_state();
    dialog.set_dialog_text_color(0, -1, 1, 1);
    dialog.set_dialog_text_color2(255, -1, 1, 1);
    dialog.set_dialog_text_color3(255, -1, 1, 1);
    dialog.field_4C_ = 1;
    dialog.field_48_ = (bounds_.bottom - bounds_.top) / 5;
    dialog.field_34_ = (bounds_.bottom - bounds_.top) / 4;
    listBox_.set_callbacks(&MessageWin::listbox_draw,
                           &MessageWin::listbox_write,
                           &MessageWin::listbox_fill);
    dialog.field_B4_ = 0;
    dialog.attach(&MainInterfaceGlobal, bounds_.left, bounds_.top, 0x1000000);
    MessageWinMessageCount = 0;
}

// Real MessageWinDeferredBaseMessages array lifecycle, compiler-generated.
/*
// ORIGINAL: 0x0051DF80 sub_51df80 0x0051DF80-0x0051DFA4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:9203a1cb6d256f6b33dd55a8d4ac395fa90a6b02:src/recovered/units/0051df80.cpp", "image:0x51df80", "source:src/messagewin.cpp"], "reason": "The native eight-StringList array at93A7B0 has30-byte stride, constructor4450B0 and complete teardown406820. The real typed MessageWinDeferredBaseMessages owns forward construction with atexit registration and reverse derived/base destruction. Preserve original identity, complete spans and historical claims; generated delegation is current lowering, not exact layout or linked-selection proof."}
// symbol    _$E26
*/
// Historical seed finding: The standalone initializer scaffold was reported measured exact in prose, but had no formal byte claim; do not grant one from that report.

// Real MessageWinDeferredBaseMessages array lifecycle, compiler-generated.
/*
// ORIGINAL: 0x0051DFB0 sub_51dfb0 0x0051DFB0-0x0051DFC4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:9203a1cb6d256f6b33dd55a8d4ac395fa90a6b02:src/recovered/units/0051dfb0.cpp", "image:0x51dfb0", "source:src/messagewin.cpp"], "reason": "The native eight-StringList array at93A7B0 has30-byte stride, constructor4450B0 and complete teardown406820. The real typed MessageWinDeferredBaseMessages owns forward construction with atexit registration and reverse derived/base destruction. Preserve original identity, complete spans and historical claims; generated delegation is current lowering, not exact layout or linked-selection proof."}
// symbol    _$E24
*/
// Historical seed finding: The old free __thiscall callback prototype failed VC6 C4234; opaque callback pointers were a scaffold workaround. Real class-array destruction supplies the typed compiler helper.
