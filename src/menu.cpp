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
#include "wave.h"
#include "original_seam.h"
#include "menu.h"
#include "pulldown.h"
#include "filewin.h"
#include "win.h"
#include "temp.h"
#include "console.h"

// RECOVERY_OBJECT: v2 0x00696E3C 0x00696E40 ?MenuSelection@@3HA 0x005FB830
int MenuSelection = -1;

// ORIGINAL: 0x005FB830 ?menu_event@Menu@@QAEHH@Z 0x005FB830-0x005FB8D8
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005FB830-0x005FB8D8"], "reason": "Recovered original behavior through real source classes and owned members; source remains subject to byte and structural verification."}
// symbol    ?menu_event@Menu@@QAEXH@Z
void Menu::menu_event(int id) {
    if (field_A24_ != 0xFFFFFFFF) {
        MenuEntry &entry = entries_[field_A24_];
        entry.pull_down->hide();
        field_A24_ = 0xFFFFFFFF;
        MenuSelection = -1;
        redraw();
        flush_input();
        do_all_tasks(entry.pull_down);
        do_all_tasks(this);
    }
    if (!(field_A28_ & 4)) hide();
    if (proc_) {
        proc_(id);
        if (win_parent_) {
            Win *target = win_parent_->event_window();
            if (target && !target->vslot_23()) target->vslot_07();
        }
    }
}

/*
Purpose: Construct the GraphicWin base and the embedded Spot, install
         Menu's own vtables, then zero the scalar fields and the 15-entry
         entries_ table.
// ORIGINAL: 0x005FAC60 ??0Menu@@QAE@XZ 0x005FAC60-0x005FACF6;0x00662CF0-0x00662D02 BYTE_EXACT
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
// LEVER: STORE ORDER inside the entries_ loop closed the last gap, and it
//        took out two divergences at once. Writing `text` before `id` (not
//        the declaration order `id`, `text`) makes VC6 aim the induction
//        pointer at `entries_[i].id` - `lea eax, [esi + 0xa38]`, stores at
//        [eax+4] then [eax] - which is what the image does; the id-first
//        spelling aimed it at `.text` and stored at [eax-4]. And the same
//        reorder stops VC6 materialising the -1 in edx (`or edx, 0xffffffff`
//        shared between field_A24_ and the fifteen `entries_[i].id`): the
//        image writes the immediate twice, once to [esi+0xa24] and once to
//        [eax] per iteration. 15/38 -> BYTE_EXACT 38/38. The earlier TRIED
//        note here said the hoisted constant was "not reachable by
//        reordering the assignments"; it was, and that note was measured
//        before the store order was tried.
Return Value: n/a
Status: Complete
*/
Menu::Menu() {
    proc_ = nullptr;
    count_ = 0;
    field_A20_ = 0;
    field_A24_ = 0xFFFFFFFF;
    for (int i = 0; i < 15; i++) {
        entries_[i].text = nullptr;
        entries_[i].id = -1;
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
// ORIGINAL: 0x005FB1D0 ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0-0x005FB224 BYTE_EXACT
// size      84 bytes
// prototype int (__thiscall ?UNK3@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8C90
Return Value: PullDown::UNK2's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original,
         and transcribed: see the BUG IN THE ORIGINAL note on the body.
*/
int Menu::UNK3(int menu_id, int item_id) {
    int index;
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own inline
    // 0xB epilogue and the break edge lands on the dead `-1` test below.
    // BYTE_EXACT here, where the range-bounded
    // `for (index = 0; index < 15; index++)` spelling left a residual range
    // compare on the found path (30 of 32) and the committed `for(;;)` plus
    // `if (++index >= 15)` form peeled the first entry read above the
    // callee-saved pushes (1 of 32).
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // (0x005FB1FC) before dispatching. The index is a loop counter that starts
    // at 0 and only increments, so this branch can never fire. Transcribed
    // anyway: it is the branch point the found path really takes, and every
    // live spelling of it (the range test) cost the two instructions. Left
    // alone deliberately - fixing it breaks the match.
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->UNK2(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB300 ?hide_menu_item@Menu@@QAEHHH@Z 0x005FB300-0x005FB354 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::hide_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::hide_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->hide_item(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB3C0 ?show_menu_item@Menu@@QAEHHH@Z 0x005FB3C0-0x005FB414 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::show_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::show_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->show_item(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB480 ?disable_menu_item@Menu@@QAEHHH@Z 0x005FB480-0x005FB4D4 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::disable_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::disable_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->disable_item(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB540 ?enable_menu_item@Menu@@QAEHHH@Z 0x005FB540-0x005FB594 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::enable_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::enable_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->enable_item(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB760 ?check_menu_item@Menu@@QAEHHH@Z 0x005FB760-0x005FB7B4 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::check_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::check_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->check_item(item_id);
}


/*
Purpose: Find the menu whose id matches and forward the item id to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB7C0 ?uncheck_menu_item@Menu@@QAEHHH@Z 0x005FB7C0-0x005FB814 BYTE_EXACT
// LEVER: the range guard at the TOP of a `for (;;)` body. VC6 rotates it
//        onto the back edge, the exhausted edge gets its own inline 0xB
//        epilogue, and the break edge lands on the dead `-1` test -
//        BYTE_EXACT. The range-bounded `for` left a residual range compare
//        on the found path (30 of 32); the committed `for(;;)` form hoisted
//        the first entry read above the callee-saved pushes (1 of 32).
//        Measured in full on ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
Return Value: PullDown::uncheck_item's, or 0xB when no entry matches
Status: Complete
Verification note: the image does test something after the loop before
         dispatching - an index compared against -1, which cannot fire for a
         counter that starts at 0 and only increments. Dead in the original.
         This tree spells the same branch point as the exhausted-range test
         `if (index >= 15)`, which is the live half of the image's two exits
         and the form that measures; see the LEVER note below.
*/
int Menu::uncheck_menu_item(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->uncheck_item(item_id);
}


/*
Purpose: Find the menu whose id matches and add a text item to its PullDown.
         An unknown menu answers 0xB, and so does a table that runs out before
         matching.
// ORIGINAL: 0x005FB100 ?add_menu_item@Menu@@QAEHHHPAD@Z 0x005FB100-0x005FB159 BYTE_EXACT
Return Value: PullDown::add_item's, or 0xB when no entry matches
Status: Complete
*/
int Menu::add_menu_item(int menu_id, int item_id, char *text) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        // The -1 sentinel ends the table early; running off the end of the
        // fifteen entries is the other way to miss. Both answer 0xB.
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: same body as the seven-clone family - the range guard at the TOP
    // of a `for (;;)` body, and the dead `-1` test below it. Written straight
    // from ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0), which holds the six spellings
    // measured for this shape.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead. Left alone deliberately.
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->add_item(item_id, text);
}


/*
Purpose: Find the menu whose id matches and append a separator to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching; a match answers 0 whatever the PullDown said.
// ORIGINAL: 0x005FB160 ?add_separator@Menu@@QAEHHH@Z 0x005FB160-0x005FB1B6 BYTE_EXACT
Return Value: 0, or 0xB when no entry matches
Status: Complete
*/
int Menu::add_separator(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead. Left alone deliberately - see
    // ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
    if (index == -1) {
        return 0xB;
    }
    // The image zeroes eax after the call: add_separator's answer is
    // discarded and this always answers 0.
    entries_[index].pull_down->add_separator(item_id);
    return 0;
}


/*
Purpose: Find the menu whose id matches and rename an item of its PullDown.
         An unknown menu answers 0xB, and so does a table that runs out before
         matching.
// ORIGINAL: 0x005FB700 ?rename_menu_item@Menu@@QAEHHHPAD@Z 0x005FB700-0x005FB759 BYTE_EXACT
Return Value: PullDown::rename_item's, or 0xB when no entry matches
Status: Complete
*/
int Menu::rename_menu_item(int menu_id, int item_id, char *text) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead. Left alone deliberately - see
    // ?UNK3@Menu@@QAEHHH@Z (0x005FB1D0).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->rename_item(item_id, text);
}


/*
Purpose: Find the menu whose id matches and answer the item index its
         PullDown holds for the item id. An unknown menu answers -1, and so
         does a table that runs out before matching.
// ORIGINAL: 0x005FB8E0 ?UNK11@Menu@@QAEHHH@Z 0x005FB8E0-0x005FB930 BYTE_EXACT
// size      80 bytes
// prototype int (__thiscall ?UNK11@Menu@@QAEHHH@Z)(Menu* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F9D00
Return Value: PullDown::id_to_index's, or -1 when no entry matches
Status: Complete
*/
int Menu::UNK11(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return -1;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return -1;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline -1 epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return -1;
    }
    return entries_[index].pull_down->id_to_index(item_id);
}


/*
Purpose: Find the menu whose id matches and answer the id its PullDown holds
         in the item slot. An unknown menu answers -1, and so does a table that
         runs out before matching.
// ORIGINAL: 0x005FB930 ?UNK12@Menu@@QAEHHH@Z 0x005FB930-0x005FB981 BYTE_EXACT
Return Value: the entry's PullDown item id, or -1 when nothing matches
Status: Complete
*/
int Menu::UNK12(int menu_id, int item_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return -1;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return -1;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard at the TOP of a `for (;;)` body, as the seven-
    // clone family (see ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0). What this body adds
    // is the slot bound, and the image merges its exit with the dead `-1`
    // test's - ONE -1 epilogue at the loop's exhausted edge, both tests
    // jumping back to it (`je 0x5fb952`, `jge 0x5fb952`). Three separate
    // `return -1;` statements did NOT merge: VC6 emitted three epilogues, one
    // spelled `or eax, eax` for the path where it knows the index is already
    // -1 (18 of 30 against this). The chained spelling is what reproduces the
    // image's shared exit - the mirror of the family's "test them separately"
    // rule, which is a rule about what the image shows, not about `||`.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so that half of the test is dead. Left alone
    // deliberately - fixing it breaks the match.
    if (index == -1 || item_id >= 64) {
        return -1;
    }
    return entries_[index].pull_down->items_[item_id].id;
}



/*
Purpose: Find the menu whose id matches and forward three arguments to its
         PullDown. An unknown menu answers 0xB, and so does a table that runs
         out before matching.
// ORIGINAL: 0x005FB240 ?UNK5@Menu@@QAEHHHHH@Z 0x005FB240-0x005FB29E BYTE_EXACT
// size      94 bytes
// prototype int (__thiscall ?UNK5@Menu@@QAEHHHHH@Z)(Menu* this, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8D90
Return Value: PullDown::UNK3's, or 0xB when no entry matches
Status: Complete
*/
int Menu::UNK5(int menu_id, int a2, int a3, int a4) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    return entries_[index].pull_down->UNK3(a2, a3, a4);
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
// ORIGINAL: 0x005FB2A0 ?UNK6@Menu@@QAEHH@Z 0x005FB2A0-0x005FB300 BYTE_EXACT
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
         path, after the flag is written. That is Win's slot 62, so the call is
         spelled `redraw()`; the ORIGINAL()/vtable-slot shim this body used
         before compiled to the same two instructions and is gone, with its
         `reinterpret_cast` of `this`.
Verification note: as in the item operations, the image's post-search test
         compares the index against -1 and cannot fire. Dead in the original,
         and transcribed: see the BUG IN THE ORIGINAL note on the body.
*/
int Menu::UNK6(int menu_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    entries_[index].flags &= 0xFE;
    redraw();
    return 0;
}


/*
Purpose: Find the menu whose id matches and set bit 0 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB360 ?UNK7@Menu@@QAEHH@Z 0x005FB360-0x005FB3C0 BYTE_EXACT
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
         path, after the flag is written. That is Win's slot 62, so the call is
         spelled `redraw()`; the ORIGINAL()/vtable-slot shim this body used
         before compiled to the same two instructions and is gone, with its
         `reinterpret_cast` of `this`.
Verification note: as in the item operations, the image's post-search test
         compares the index against -1 and cannot fire. Dead in the original,
         and transcribed: see the BUG IN THE ORIGINAL note on the body.
*/
int Menu::UNK7(int menu_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    entries_[index].flags |= 0x01;
    redraw();
    return 0;
}


/*
Purpose: Find the menu whose id matches and set bit 1 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB420 ?UNK8@Menu@@QAEHH@Z 0x005FB420-0x005FB480 BYTE_EXACT
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
         path, after the flag is written. That is Win's slot 62, so the call is
         spelled `redraw()`; the ORIGINAL()/vtable-slot shim this body used
         before compiled to the same two instructions and is gone, with its
         `reinterpret_cast` of `this`.
Verification note: as in the item operations, the image's post-search test
         compares the index against -1 and cannot fire. Dead in the original,
         and transcribed: see the BUG IN THE ORIGINAL note on the body.
*/
int Menu::UNK8(int menu_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    entries_[index].flags |= 0x02;
    redraw();
    return 0;
}


/*
Purpose: Find the menu whose id matches and clear bit 1 of its entry's flag byte,
         then let the window repaint itself. An unknown menu answers 0xB.
// ORIGINAL: 0x005FB4E0 ?UNK9@Menu@@QAEHH@Z 0x005FB4E0-0x005FB540 BYTE_EXACT
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
         path, after the flag is written. That is Win's slot 62, so the call is
         spelled `redraw()`; the ORIGINAL()/vtable-slot shim this body used
         before compiled to the same two instructions and is gone, with its
         `reinterpret_cast` of `this`.
Verification note: as in the item operations, the image's post-search test
         compares the index against -1 and cannot fire. Dead in the original,
         and transcribed: see the BUG IN THE ORIGINAL note on the body.
*/
int Menu::UNK9(int menu_id) {
    int index;
    for (index = 0;; index++) {
        if (index >= 15) {
            return 0xB;
        }
        const int id = entries_[index].id;
        if (id == -1) {
            return 0xB;
        }
        if (id == menu_id) {
            break;
        }
    }
    // LEVER: the range guard written at the TOP of a `for (;;)` body, not as
    // the loop's own second clause. VC6 rotates it onto the back edge
    // (`cmp eax, 0xf` / `jl`), so the exhausted edge falls into its own
    // inline 0xB epilogue and the break edge lands on the dead `-1` test
    // the image carries below. The range-bounded
    // `for (index = 0; index < 15; index++)` spelling left the residual
    // range compare on the found path instead - 30 of 32 against
    // BYTE_EXACT here.
    // BUG IN THE ORIGINAL: the image re-tests the index against -1 here
    // before dispatching. The index is a loop counter that starts at 0 and
    // only increments, so the branch is dead - it never fires. Transcribed
    // anyway: it is the branch point the found path really takes, and
    // spelling it as the live range test cost two instructions (see
    // ?UNK3@Menu@@QAEHHH@Z 0x005FB1D0 for the six spellings measured).
    if (index == -1) {
        return 0xB;
    }
    entries_[index].flags &= 0xFD;
    redraw();
    return 0;
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

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Wave g_MENU_UP_WAVE;  // 0x00749C88
Wave g_MENU_DOWN_WAVE;  // 0x0074D960
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
