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
#include "menu.h"
#include "pulldown.h"

/*
Purpose: Set the callback invoked for menu events.
Original Offset: 005FB820
Status: Complete
*/
void Menu::set_menu_proc(MenuProc proc) {
    proc_ = proc;
}

/*
Purpose: Find the first menu entry with the requested ID.
Original Offset: 005FB990
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
Original Offset: 005FB1C0
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
Original Offset: 005FB230
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
Original Offset: 005FC6A0
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
Original Offset: 005FB1D0
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
Original Offset: 005FB300
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
Original Offset: 005FB3C0
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
Original Offset: 005FB480
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
Original Offset: 005FB540
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
Original Offset: 005FB760
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
Original Offset: 005FB7C0
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
