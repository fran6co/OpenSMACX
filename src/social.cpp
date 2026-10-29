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
#include "socialwin.h"
#include "tutwin.h"
#include "game.h"
#include "game_data_state_window.h"
#include "console.h"
#include "netdaemon.h"
#include "faction.h"

// ORIGINAL: 0x005B5620 ?social_select@@YAXH@Z 0x005B5620-0x005B5688 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/005b5620.cpp retained original identity/extent; pinned instructions5B5620..5B5688 establish flags, typed receivers and ordered calls; recovery/tests/test_social_select_behavior.py actual-source controls", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/005b5620.cpp"], "reason": "SocialWin owner opens the real shared window, synchronizes choices and applies them offline, preserving native entry-state guards and editor selection. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// TRIED: homed into general.cpp 2026-09-03 - does not compile: the body calls
void __cdecl social_select(int faction_id) {
    const uint8_t state = static_cast<uint8_t>(GameDataStateStorage.fields_.game_state_);
    if ((state & (STATE_SCENARIO_EDITOR | STATE_OMNISCIENT_VIEW)) == 0 &&
        (GameDataStateStorage.fields_.net_turn_flags_ & 8u) != 0)
        return;

    int selected_faction;
    if ((state & STATE_OMNISCIENT_VIEW) != 0) {
        selected_faction = ConsoleGlobal.editor_who(faction_id, 1);
        if (selected_faction < 0)
            return;
    } else {
        selected_faction = faction_id;
    }

    TutWinSocWindow.init(selected_faction);
    TutWinSocWindow.close();
    synch_soc(selected_faction);
    if (IsMultiplayerNet == 0)
        social_set(selected_faction);
}
