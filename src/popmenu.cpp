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
        char *text = items.head_ ? items.current_->payload->text : 0;
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
