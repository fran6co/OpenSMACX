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

/*
Purpose: Set the callback invoked for menu events.
ORIGINAL: 0x005FB820 BYTE_EXACT
// name      ?set_menu_proc@Menu@@QAEXP6AXH@Z@Z
// size      13 bytes
// spans     0x005FB820-0x005FB82D
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
ORIGINAL: 0x005FB990 BYTE_EXACT
// name      ?id_to_index@Menu@@QAEHH@Z
// size      40 bytes
// spans     0x005FB990-0x005FB9B8
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
ORIGINAL: 0x005FB1C0 BYTE_EXACT
// name      ?UNK2@Menu@@QAEHH@Z
// size      5 bytes
// spans     0x005FB1C0-0x005FB1C5
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
ORIGINAL: 0x005FB230 BYTE_EXACT
// name      ?UNK4@Menu@@QAEHHHH@Z
// size      5 bytes
// spans     0x005FB230-0x005FB235
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

Font **MenuFont = reinterpret_cast<Font **>(0x009B7B94);

/*
Purpose: Report the height a menu requests, two lines of its font.
ORIGINAL: 0x005FC6A0
// name      ?requested_height@Menu@@QAEHXZ
// size      20 bytes
// spans     0x005FC6A0-0x005FC6B4
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
    Font *font = *MenuFont;
    if (!font) {
        font = *FontDefaultPtr;
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
ORIGINAL: 0x005FB1D0
// name      ?UNK3@Menu@@QAEHHH@Z
// size      84 bytes
// spans     0x005FB1D0-0x005FB224
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
ORIGINAL: 0x005FB300
// name      ?hide_menu_item@Menu@@QAEHHH@Z
// size      84 bytes
// spans     0x005FB300-0x005FB354
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
ORIGINAL: 0x005FB3C0
// name      ?show_menu_item@Menu@@QAEHHH@Z
// size      84 bytes
// spans     0x005FB3C0-0x005FB414
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
ORIGINAL: 0x005FB480
// name      ?disable_menu_item@Menu@@QAEHHH@Z
// size      84 bytes
// spans     0x005FB480-0x005FB4D4
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
ORIGINAL: 0x005FB540
// name      ?enable_menu_item@Menu@@QAEHHH@Z
// size      84 bytes
// spans     0x005FB540-0x005FB594
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
ORIGINAL: 0x005FB760
// name      ?check_menu_item@Menu@@QAEHHH@Z
// size      84 bytes
// spans     0x005FB760-0x005FB7B4
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
ORIGINAL: 0x005FB7C0
// name      ?uncheck_menu_item@Menu@@QAEHHH@Z
// size      84 bytes
// spans     0x005FB7C0-0x005FB814
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
ORIGINAL: 0x005FB2A0
// name      ?UNK6@Menu@@QAEHH@Z
// size      96 bytes
// spans     0x005FB2A0-0x005FB300
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
ORIGINAL: 0x005FB360
// name      ?UNK7@Menu@@QAEHH@Z
// size      96 bytes
// spans     0x005FB360-0x005FB3C0
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
ORIGINAL: 0x005FB420
// name      ?UNK8@Menu@@QAEHH@Z
// size      96 bytes
// spans     0x005FB420-0x005FB480
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
ORIGINAL: 0x005FB4E0
// name      ?UNK9@Menu@@QAEHH@Z
// size      96 bytes
// spans     0x005FB4E0-0x005FB540
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
ORIGINAL: 0x005FC6C0 BYTE_EXACT
// name      ?on_adjust_pulldown_pos@Menu@@QAEXPAH0@Z
// size      3 bytes
// spans     0x005FC6C0-0x005FC6C3
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
ORIGINAL: 0x005FC680 BYTE_EXACT
// name      ?on_mouse_move@Menu@@QAEXHH@Z
// size      18 bytes
// spans     0x005FC680-0x005FC692
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

func_menu_mouse_move MenuMouseMove =
    original_method<func_menu_mouse_move>(0x005FBDB0);

/*
Purpose: Track the pointer over the menu bar. The body at 0x005FBDB0 is NOT
         recovered; this is a seam to the original image, not a recovery, and
         deliberately carries no `Original Offset:` line so the catalogue does
         not mistake it for one.
Status: Forwarded to the original image
*/
void Menu::mouse_move(int a1, int a2) {
    (ORIGINAL(this)->*MenuMouseMove)(a1, a2);
}
