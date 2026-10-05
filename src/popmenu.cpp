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
#include "popmenu.h"
#include "dialog.h"
#include "sounddevice.h"
#include "basepop.h"

// PullDown is an owned member; Popup is the base. Reverse declaration order
// supplies both normal destruction and exception cleanup exactly once.
// ORIGINAL: 0x00421400 ??1PopMenu@@QAE@XZ 0x00421400-0x00421634;0x00652AB0-0x00652B9F
// RECOVERY_BINDING: v1 {"evidence": ["image:0x421400"], "reason": "Recover the full original operation using owned C++ members and base lifetimes; retain all original branches and expose unresolved callees instead of executing original-address forwarders."}
// symbol    ??1PopMenu@@UAE@XZ
PopMenu::~PopMenu() {
}

// ORIGINAL: 0x0059D250 ?exec@PopMenu@@QAEXHHP6AHXZ@Z 0x0059D250-0x0059D392
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0059D250-0x0059D392"], "reason": "Recovered original behavior through real source classes and owned members; source remains subject to byte and structural verification."}
// symbol    ?exec@PopMenu@@QAEHHHP6AHXZ@Z
int PopMenu::exec(int x, int y, int (*poll)()) {
    pull_down_.init(0);
    for (int i = 0; i < dialogs().get_num_items(); ++i) {
        StringList &items = dialogs().item_list_;
        items.seek_pos(i);
        char *text = items.head_ ? items.current_->string_payload()->text : 0;
        int id = items.head_ ? items.current_->id : 0;
        // Dialog menu separator literal at 0x006900C8.
        if (strcmp(text, "*") == 0) pull_down_.add_separator(id);
        else pull_down_.add_item(id, text);
    }
    pull_down_.show(x, y, poll);
    return pull_down_.get_selected();
}


/*
Purpose: Initialise the pop-up menu with the base defaults.
// ORIGINAL: 0x0059D3A0 ?init@PopMenu@@QAEHXZ 0x0059D3A0-0x0059D3AA BYTE_EXACT
// size      10 bytes
// prototype int (__thiscall ?init@PopMenu@@QAEHXZ)(PopMenu* this)
// callers   14   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006015B0
Return Value: whatever BasePop::init returns
Status: Complete
*/
int PopMenu::init() {
    return BasePop::init(0, 0);
}

// The original frame embeds PullDown at Popup+0x537C and unwinds the same
// PopMenu at 0x006612F3: construction and teardown belong to one local object.
// ORIGINAL: 0x0059D690 ?popm@@YAHPADHHP6AHXZ@Z 0x0059D690-0x0059D971;0x006612E8-0x006613D0
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0059d690.cpp", "source:src/popmenu.h", "image:0x59d690"], "reason": "Recover the complete original operation through genuine owning C++ classes, members and natural lifetimes; preserve retained seed evidence and historical restoration obligations separately from fresh matching."}
int __cdecl popm(char *section, int x, int y, int (__cdecl *poll)()) {
    PopMenu menu;
    menu.start(PopupStartCaption, section, -1, 0, 0, 0);
    return menu.exec(x, y, poll);
}

// Superseded seed removed; retained findings below are historical evidence.
// ORIGINAL: 0x0059D3B0 ?popm@@YAHPADPADHHP6AHXZ@Z 0x0059D3B0-0x0059D690;0x00661200-0x006612E8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:67e9243d4c82aaa8b046653cd25ad7281d0e1bef:src/unrecovered/0059d3b0.cpp", "pinned:terranx_original:0x59d3b0", "source:src/popmenu.cpp"], "reason": "Recover distinct file-and-section PopMenu overload through one actual PopMenu local and member-before-base cleanup. Preserve complete spans, original identity and historical obligations; scoped compiler/host observations are not game acceptance."}
// symbol    ?popm@@YAHPAD0HHP6AHXZ@Z
// TRIED: PopMenu as a bare (non-Popup-derived) class per the scaffold - the disasm constructs a `Popup` at the exact address later used for `PopMenu::exec`, so PopMenu : public Popup; exec() returns int (eax is captured into esi and returned), not the scaffold's void. Landed as plain RAII locals (PopMenu menu; PullDown pulldown;) with declared-only ~PopMenu/~BasePop/~FlatButton so the TU doesn't synthesize+define them itself (which trips the "one external .text symbol" gate). This gets the SEH frame, the state-index cleanup counter and the visible per-member close()/dtor chain (~968 bytes) nowhere near matched: the original inlines BasePop's/Popup's own dtor bodies directly into popm(), which needs those functions' real source (not just their addresses) to reproduce - out of reach here.
int __cdecl popm(char *text_file, char *text_label, int x, int y,
                 int (__cdecl *poll)()) {
    PopMenu menu;
    menu.start(text_file, text_label, -1, 0, 0, 0);
    return menu.exec(x, y, poll);
}

// Historical seed lifetime hypotheses used duplicate locals and raw placement storage.
// Native receiver evidence supersedes those models; genuine embedded member lifetimes now own cleanup.
