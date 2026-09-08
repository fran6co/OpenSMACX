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
