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
#include "menu.h"
#include "pulldown.h"
#include "filewin.h"
#include "win.h"

const uint32_t MenuPrimaryVtable = 0x006700B0;
const uint32_t MenuBufferVtable = 0x006700A8;

/*
Purpose: Construct the GraphicWin base and the embedded Spot, install
         Menu's own vtables, then zero the scalar fields and the 15-entry
         entries_ table.
// ORIGINAL: 0x005FAC60 ??0Menu@@QAE@XZ 0x005FAC60-0x005FACF6;0x00662CF0-0x00662D02
// size      150 bytes
// prototype void (__thiscall ??0Menu@@QAE@XZ)(Menu* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005D4CF0 0x005FA860
// notes     field_A1C_ and field_A28_ are never touched by this constructor
//        (the image writes proc_, count_, field_A20_ and field_A24_ only).
// LEVER: a REAL CONSTRUCTOR, not a `construct()` method. This was
//        `Menu *Menu::construct()` doing `new (&spot_) Spot()`, and the
//        placement-new is what cost it. VC6 guards a placement new-expression
//        with a null test on the pointer - `cmp ecx, ebx; je` - because
//        `operator new` may return null, and the guard needs a spill slot.
//        That is where `sub esp, 8` came from against the image's `push ecx`.
//        The earlier note called this "a stack-frame-size difference from
//        spill-slot count, not control flow", which had the cause backwards:
//        there IS control flow, and the frame size is downstream of it.
//        As an ordinary member of a real constructor, `spot_` is constructed
//        implicitly with no guard at all - and the image agrees, calling
//        GraphicWin's real constructor at 0x005D4CF0 and then Spot's at
//        0x005FA860, which is exactly base-then-member in declaration order.
//        10/38 -> 15/38, and the compiled body drops from 42 instructions to
//        39 against the image's 38.
// TRIED: the last gap is one hoisted constant. VC6 puts -1 in edx
//        (`or edx, 0xffffffff`) because `field_A24_` and the fifteen
//        `entries_[i].id` share it, where the image writes the immediate
//        straight to `[esi + 0xa24]`. Not reachable by reordering the
//        assignments; not chased further.
Return Value: n/a
Status: Complete
*/
Menu::Menu() {
    proc_ = nullptr;
    count_ = 0;
    field_A20_ = 0;
    field_A24_ = 0xFFFFFFFF;
    for (int i = 0; i < 15; i++) {
        entries_[i].id = -1;
        entries_[i].text = nullptr;
        entries_[i].flags = 0;
        entries_[i].mnemonic = nullptr;
        entries_[i].pull_down = nullptr;
    }
}

/*
Purpose: Set the callback invoked for menu events.
// ORIGINAL: 0x005FB820 ?set_menu_proc@Menu@@QAEXP6AXH@Z@Z 0x005FB820-0x005FB82D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_menu_proc@Menu@@QAEXP6AXH@Z@Z)(Menu* this, void (__cdecl *)(int this))
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Menu::set_menu_proc(MenuProc proc) {
    proc_ = proc;
}

/*
Purpose: Find the first menu entry with the requested ID.
// ORIGINAL: 0x005FB990 ?id_to_index@Menu@@QAEHH@Z 0x005FB990-0x005FB9B8 BYTE_EXACT
// size      40 bytes
// prototype int (__thiscall ?id_to_index@Menu@@QAEHH@Z)(Menu* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int Menu::id_to_index(int id) {
    for (int index = 0; index < 15; ++index) {
        if (entries_[index].id == -1) {
            break;
        }
        if (entries_[index].id == id) {
            return index;
        }
    }
    return -1;
}

MenuProc __fastcall menu_set_menu_proc_redirect(
        Menu *self, void *, MenuProc proc) {
    self->set_menu_proc(proc);
    return proc;
}

int __fastcall menu_id_to_index_redirect(Menu *self, void *, int id) {
    return self->id_to_index(id);
}

/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005FB1C0 ?UNK2@Menu@@QAEHH@Z 0x005FB1C0-0x005FB1C5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK2@Menu@@QAEHH@Z)(Menu* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Menu::UNK2(int) {
    return 0;
}

int __fastcall menu_unk2_redirect(
        Menu *self, void *, int a) {
    return self->UNK2(a);
}

/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005FB230 ?UNK4@Menu@@QAEHHHH@Z 0x005FB230-0x005FB235 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK4@Menu@@QAEHHHH@Z)(Menu* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Menu::UNK4(int, int, int) {
    return 0;
}

int __fastcall menu_unk4_redirect(
        Menu *self, void *, int a, int b, int c) {
    return self->UNK4(a, b, c);
}


/*
Purpose: Report the height a menu requests, two lines of its font.
// ORIGINAL: 0x005FC6A0 ?requested_height@Menu@@QAEHXZ 0x005FC6A0-0x005FC6B4 BYTE_EXACT
// size      20 bytes
// prototype int (__thiscall ?requested_height@Menu@@QAEHXZ)(Menu* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Twice the resolved font's height
Status: Complete
*/
int Menu::requested_height() {
    // The menu's own font wins when set; otherwise the process default is
    // resolved at the point of use rather than cached.
    Font *font = MenuFont();
    if (!font) {
        font = FontDefault;
    }
    return font->height_ * 2;
}

int __fastcall menu_requested_height_redirect(Menu *self, void *) {
    return self->requested_height();
}

/*
 * The seven menu-item operations below are ONE function seven times over: the
 * image holds them as 84-byte clones differing at exactly two byte offsets,
 * the displacement of the single call at the end. Everything else - the
 * inlined search, the two 0xB exits, the entry stride - is byte-identical
 * across all seven, which is why they are transcribed as a family rather than
 * derived one at a time.
 *
 * The search is INLINED in each, not delegated to Menu::id_to_index, because
 * that is what the original does: id_to_index exists separately at 0x005FB280
 * and every one of these carries its own copy of the same loop. Factoring
 * them onto a shared helper would leave the mutation sweep with one body to
 * perturb instead of seven.
 */

/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB1D0 ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0-0x005FB224
// size      84 bytes
// prototype int (__thiscall ?UNK3@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8C90
Return Value: PullDown::UNK2's, or 0xB when no entry matches
Status: Complete
Verification note: the original checks the found index against -1 at
         0x005FB1FC before dispatching, which can never fire - the index
         is a loop counter that starts at 0 and only increments. Dead in the
         original and omitted here rather than transcribed as an unreachable
         branch.
*/
int Menu::UNK3(int menu_id, int item_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    return entries_[index].pull_down->UNK2(item_id);
}

int __fastcall menu_unk3_redirect(
        Menu *self, void *, int menu_id, int item_id) {
    return self->UNK3(menu_id, item_id);
}

/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB300 ?hide_menu_item@Menu@@QAEHHH@Z 0x005FB300-0x005FB354
// TRIED: shared defect across the whole seven-clone family (UNK3,
//        hide/show/disable/enable/check/uncheck_menu_item all measure
//        1/32, 0.754 similar at their best flag set). Every one of them
//        diverges starting at INSTRUCTION 0: this tree's compile hoists a
//        read of entries_[0].id (`mov eax,[ecx+0xa38]`) and the loop-index
//        zeroing ahead of the `push esi`/`push edi` prologue and the
//        edi=menu_id load, which the image never does - VC6 loop-rotating
//        the `for(;;) { if(first-check) ...}` shape to peel the first
//        iteration's read before the callee-saved pushes. Tried: a genuine
//        `do { } while(++index < 15)` (worse, 1/32 best, 0.746 similar -
//        still peels); a raw `MenuEntry *entry` pointer walk instead of
//        `entries_[index]` (worse, 0/32 at every flag set tried, still
//        peels and also loses the /Oy- frame). `--all-flags` on the
//        committed `for(;;)` form plateaus at 5/32 (0.738) with /Oy- kept,
//        1/32 (0.754, best similarity) with it omitted - the hoist happens
//        at every flag set tried. Not a source-shape lever found so far.
// size      84 bytes
// prototype int (__thiscall ?hide_menu_item@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   2   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005F8CB0
Return Value: PullDown::hide_item's, or 0xB when no entry matches
Status: Complete
Verification note: the original checks the found index against -1 at
         0x005FB32C before dispatching, which can never fire - the index
         is a loop counter that starts at 0 and only increments. Dead in the
         original and omitted here rather than transcribed as an unreachable
         branch.
*/
int Menu::hide_menu_item(int menu_id, int item_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    return entries_[index].pull_down->hide_item(item_id);
}

int __fastcall menu_hide_menu_item_redirect(
        Menu *self, void *, int menu_id, int item_id) {
    return self->hide_menu_item(menu_id, item_id);
}

/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB3C0 ?show_menu_item@Menu@@QAEHHH@Z 0x005FB3C0-0x005FB414
// TRIED: shared defect across the whole seven-clone family (UNK3,
//        hide/show/disable/enable/check/uncheck_menu_item all measure
//        1/32, 0.754 similar at their best flag set) - see
//        ?hide_menu_item@Menu@@QAEHHH@Z (0x005FB300) for the full
//        investigation (loop-rotation hoist, do/while and raw-pointer
//        forms both tried and both worse). Not repeated per-clone.
// size      84 bytes
// prototype int (__thiscall ?show_menu_item@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8D20
Return Value: PullDown::show_item's, or 0xB when no entry matches
Status: Complete
Verification note: the original checks the found index against -1 at
         0x005FB3EC before dispatching, which can never fire - the index
         is a loop counter that starts at 0 and only increments. Dead in the
         original and omitted here rather than transcribed as an unreachable
         branch.
*/
int Menu::show_menu_item(int menu_id, int item_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    return entries_[index].pull_down->show_item(item_id);
}

int __fastcall menu_show_menu_item_redirect(
        Menu *self, void *, int menu_id, int item_id) {
    return self->show_menu_item(menu_id, item_id);
}

/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB480 ?disable_menu_item@Menu@@QAEHHH@Z 0x005FB480-0x005FB4D4
// TRIED: same shared-family defect as ?hide_menu_item@Menu@@QAEHHH@Z
//        (0x005FB300, see its note) - a `do { } while(++index < 15)` and a
//        raw `MenuEntry *` pointer walk were both tried there and both
//        measured worse than the committed `for(;;)` form; not repeated
//        per-clone. Plateaus at 1/32, 0.754 similar.
// size      84 bytes
// prototype int (__thiscall ?disable_menu_item@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8D90
Return Value: PullDown::disable_item's, or 0xB when no entry matches
Status: Complete
Verification note: the original checks the found index against -1 at
         0x005FB4AC before dispatching, which can never fire - the index
         is a loop counter that starts at 0 and only increments. Dead in the
         original and omitted here rather than transcribed as an unreachable
         branch.
*/
int Menu::disable_menu_item(int menu_id, int item_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    return entries_[index].pull_down->disable_item(item_id);
}

int __fastcall menu_disable_menu_item_redirect(
        Menu *self, void *, int menu_id, int item_id) {
    return self->disable_menu_item(menu_id, item_id);
}

/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB540 ?enable_menu_item@Menu@@QAEHHH@Z 0x005FB540-0x005FB594
// TRIED: shared defect across the whole seven-clone family (UNK3,
//        hide/show/disable/enable/check/uncheck_menu_item all measure
//        1/32, 0.754 similar at their best flag set) - see
//        ?hide_menu_item@Menu@@QAEHHH@Z (0x005FB300) for the full
//        investigation (loop-rotation hoist, do/while and raw-pointer
//        forms both tried and both worse). Not repeated per-clone.
// size      84 bytes
// prototype int (__thiscall ?enable_menu_item@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8DF0
Return Value: PullDown::enable_item's, or 0xB when no entry matches
Status: Complete
Verification note: the original checks the found index against -1 at
         0x005FB56C before dispatching, which can never fire - the index
         is a loop counter that starts at 0 and only increments. Dead in the
         original and omitted here rather than transcribed as an unreachable
         branch.
*/
int Menu::enable_menu_item(int menu_id, int item_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    return entries_[index].pull_down->enable_item(item_id);
}

int __fastcall menu_enable_menu_item_redirect(
        Menu *self, void *, int menu_id, int item_id) {
    return self->enable_menu_item(menu_id, item_id);
}

/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB760 ?check_menu_item@Menu@@QAEHHH@Z 0x005FB760-0x005FB7B4
// TRIED: shared defect across the whole seven-clone family (UNK3,
//        hide/show/disable/enable/check/uncheck_menu_item all measure
//        1/32, 0.754 similar at their best flag set) - see
//        ?hide_menu_item@Menu@@QAEHHH@Z (0x005FB300) for the full
//        investigation (loop-rotation hoist, do/while and raw-pointer
//        forms both tried and both worse). Not repeated per-clone.
// size      84 bytes
// prototype int (__thiscall ?check_menu_item@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   5   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005F9040
Return Value: PullDown::check_item's, or 0xB when no entry matches
Status: Complete
Verification note: the original checks the found index against -1 at
         0x005FB78C before dispatching, which can never fire - the index
         is a loop counter that starts at 0 and only increments. Dead in the
         original and omitted here rather than transcribed as an unreachable
         branch.
*/
int Menu::check_menu_item(int menu_id, int item_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    return entries_[index].pull_down->check_item(item_id);
}

int __fastcall menu_check_menu_item_redirect(
        Menu *self, void *, int menu_id, int item_id) {
    return self->check_menu_item(menu_id, item_id);
}

/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB7C0 ?uncheck_menu_item@Menu@@QAEHHH@Z 0x005FB7C0-0x005FB814
// TRIED: shared defect across the whole seven-clone family (UNK3,
//        hide/show/disable/enable/check/uncheck_menu_item all measure
//        1/32, 0.754 similar at their best flag set) - see
//        ?hide_menu_item@Menu@@QAEHHH@Z (0x005FB300) for the full
//        investigation (loop-rotation hoist, do/while and raw-pointer
//        forms both tried and both worse). Not repeated per-clone.
// size      84 bytes
// prototype int (__thiscall ?uncheck_menu_item@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F90A0
Return Value: PullDown::uncheck_item's, or 0xB when no entry matches
Status: Complete
Verification note: the original checks the found index against -1 at
         0x005FB7EC before dispatching, which can never fire - the index
         is a loop counter that starts at 0 and only increments. Dead in the
         original and omitted here rather than transcribed as an unreachable
         branch.
*/
int Menu::uncheck_menu_item(int menu_id, int item_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    return entries_[index].pull_down->uncheck_item(item_id);
}

int __fastcall menu_uncheck_menu_item_redirect(
        Menu *self, void *, int menu_id, int item_id) {
    return self->uncheck_menu_item(menu_id, item_id);
}

/*
 * A second Menu clone family: four 96-byte bodies differing at two byte
 * offsets, which are the flag instruction itself - `and dl, 0xFE`,
 * `or dl, 1`, `or dl, 2`, `and dl, 0xFD`. Two bits, set and cleared. As with
 * the item operations above, the search is inlined in each rather than
 * delegated, because the original carries four copies of it.
 */

/*
Purpose: Find the menu whose id matches and clear bit 0 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB2A0 ?UNK6@Menu@@QAEHH@Z 0x005FB2A0-0x005FB300
// size      96 bytes
// prototype int (__thiscall ?UNK6@Menu@@QAEHH@Z)(Menu* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x005FB2F3
Return Value: 0 once the flag is written, 0xB when no entry matches
Status: Complete
Verification note: the repaint is a virtual dispatch on THIS object's own
         vtable - `mov edx, [ecx]` / `call [edx + 0xF8]` - not on the entry or
         its PullDown, and it takes no arguments. It runs only on the found
         path, after the flag is written.
Verification note: as in the item operations, the original tests the found
         index against -1 before using it, which cannot fire for a counter
         that starts at 0 and only increments. Dead, and omitted.
*/
int Menu::UNK6(int menu_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    entries_[index].flags &= 0xFE;
    void **const vtable = *reinterpret_cast<void ***>(this);
    (ORIGINAL(this)->*original_method<func_menu_repaint>(reinterpret_cast<unsigned long>(vtable[0xF8 / sizeof(void *)])))();
    return 0;
}

int __fastcall menu_unk6_redirect(
        Menu *self, void *, int menu_id) {
    return self->UNK6(menu_id);
}

/*
Purpose: Find the menu whose id matches and set bit 0 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB360 ?UNK7@Menu@@QAEHH@Z 0x005FB360-0x005FB3C0
// size      96 bytes
// prototype int (__thiscall ?UNK7@Menu@@QAEHH@Z)(Menu* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005FB3B3
Return Value: 0 once the flag is written, 0xB when no entry matches
Status: Complete
Verification note: the repaint is a virtual dispatch on THIS object's own
         vtable - `mov edx, [ecx]` / `call [edx + 0xF8]` - not on the entry or
         its PullDown, and it takes no arguments. It runs only on the found
         path, after the flag is written.
Verification note: as in the item operations, the original tests the found
         index against -1 before using it, which cannot fire for a counter
         that starts at 0 and only increments. Dead, and omitted.
*/
int Menu::UNK7(int menu_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    entries_[index].flags |= 0x01;
    void **const vtable = *reinterpret_cast<void ***>(this);
    (ORIGINAL(this)->*original_method<func_menu_repaint>(reinterpret_cast<unsigned long>(vtable[0xF8 / sizeof(void *)])))();
    return 0;
}

int __fastcall menu_unk7_redirect(
        Menu *self, void *, int menu_id) {
    return self->UNK7(menu_id);
}

/*
Purpose: Find the menu whose id matches and set bit 1 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB420 ?UNK8@Menu@@QAEHH@Z 0x005FB420-0x005FB480
// size      96 bytes
// prototype int (__thiscall ?UNK8@Menu@@QAEHH@Z)(Menu* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005FB473
Return Value: 0 once the flag is written, 0xB when no entry matches
Status: Complete
Verification note: the repaint is a virtual dispatch on THIS object's own
         vtable - `mov edx, [ecx]` / `call [edx + 0xF8]` - not on the entry or
         its PullDown, and it takes no arguments. It runs only on the found
         path, after the flag is written.
Verification note: as in the item operations, the original tests the found
         index against -1 before using it, which cannot fire for a counter
         that starts at 0 and only increments. Dead, and omitted.
*/
int Menu::UNK8(int menu_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    entries_[index].flags |= 0x02;
    void **const vtable = *reinterpret_cast<void ***>(this);
    (ORIGINAL(this)->*original_method<func_menu_repaint>(reinterpret_cast<unsigned long>(vtable[0xF8 / sizeof(void *)])))();
    return 0;
}

int __fastcall menu_unk8_redirect(
        Menu *self, void *, int menu_id) {
    return self->UNK8(menu_id);
}

/*
Purpose: Find the menu whose id matches and clear bit 1 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB4E0 ?UNK9@Menu@@QAEHH@Z 0x005FB4E0-0x005FB540
// size      96 bytes
// prototype int (__thiscall ?UNK9@Menu@@QAEHH@Z)(Menu* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x005FB533
Return Value: 0 once the flag is written, 0xB when no entry matches
Status: Complete
Verification note: the repaint is a virtual dispatch on THIS object's own
         vtable - `mov edx, [ecx]` / `call [edx + 0xF8]` - not on the entry or
         its PullDown, and it takes no arguments. It runs only on the found
         path, after the flag is written.
Verification note: as in the item operations, the original tests the found
         index against -1 before using it, which cannot fire for a counter
         that starts at 0 and only increments. Dead, and omitted.
*/
int Menu::UNK9(int menu_id) {
    int index = 0;
    for (;;) {
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
        if (++index >= 15) {
            return 0xB;
        }
    }
    entries_[index].flags &= 0xFD;
    void **const vtable = *reinterpret_cast<void ***>(this);
    (ORIGINAL(this)->*original_method<func_menu_repaint>(reinterpret_cast<unsigned long>(vtable[0xF8 / sizeof(void *)])))();
    return 0;
}

int __fastcall menu_unk9_redirect(
        Menu *self, void *, int menu_id) {
    return self->UNK9(menu_id);
}

/*
Purpose: Legacy hook for adjusting a pull-down's position. The original body is
         a single `ret 8`: it accepts two pointers, reads neither, writes
         neither, and returns. Recovered as an empty body rather than left
         unrecovered, because "does nothing" is a behaviour the catalogue can
         own and a trap cannot.

         The mangled name and the stack cleanup AGREE here, which is why this
         one is safe where ?on_redraw@Win@@QAEHXZ beside it in the queue is
         not: on_redraw declares no parameters and still cleans eight bytes, so
         its arity is unresolved and it stays unrecovered until that is
         settled.
// ORIGINAL: 0x005FC6C0 ?on_adjust_pulldown_pos@Menu@@QAEXPAH0@Z 0x005FC6C0-0x005FC6C3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_adjust_pulldown_pos@Menu@@QAEXPAH0@Z)(Menu* this, int*, int*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Menu::on_adjust_pulldown_pos(int *, int *) {
}

void __fastcall menu_on_adjust_pulldown_pos_redirect(Menu *self, void *, int *a1, int *a2) {
    self->on_adjust_pulldown_pos(a1, a2);
}

/*
// ORIGINAL: 0x005FC680 ?on_mouse_move@Menu@@QAEXHH@Z 0x005FC680-0x005FC692 BYTE_EXACT
// size      18 bytes
// prototype void (__thiscall ?on_mouse_move@Menu@@QAEXHH@Z)(Menu* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FBDB0
Status: Complete
*/
void Menu::on_mouse_move(int a1, int a2) {
    mouse_move(a1, a2);
}

/*
Purpose: Step the receiver back to the subobject ??_GMenu@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x005FC6F0 ??_GMenu@@WEEE@AEPAXI@Z 0x005FC6F0-0x005FC6FB BYTE_EXACT
// symbol    ??_EMenu@@WEEE@AEPAXI@Z
// CORRECTED from ??3Menu@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005FC6D0` into ??_GMenu@@UAEPAXI@Z,
//   which executes `ret 4`; no stack access and the receiver stays in
//   ECX. `WEEE@` re-demangles to adjustor{1092} and 1092 == 0x444, the
//   constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
