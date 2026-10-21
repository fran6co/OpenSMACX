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
#include "faction.h"
#include "netdaemon.h"
#include "log.h"

/*
Purpose: Apply a treaty between two factions, or - in a network game - log
         it, broadcast it and wait for the receiving side's ack instead.
// ORIGINAL: 0x005391C0 ?net_treaty_on@@YAXHHHH@Z 0x005391C0-0x0053922A BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?net_treaty_on@@YAXHHHH@Z)(int, int, int, int)
// callers   4   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055B870 0x00592EE0 0x006262F0
// LEVER: `IsMultiplayerNet` for 0x0093F660 and `NetDaemonNet` for the
//        0x0093CD90 receiver - the artifact's fixed-address casts lower to
//        the same two immediates once the names are real objects. Homed
//        from the recovered archive.
Status: Complete
*/
void __cdecl net_treaty_on(int faction_id, int faction_id_with, int treaty,
                           int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYON, faction_id, faction_id_with, treaty);
        message_data(0x2441, 0, faction_id, faction_id_with, treaty, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x441);
        }
    } else {
        treaty_on(faction_id, faction_id_with, treaty);
    }
}
/*
Purpose: net_treaty_on's mirror - clear a treaty, or broadcast the clearing.
// ORIGINAL: 0x00539230 ?net_treaty_off@@YAXHHHH@Z 0x00539230-0x0053929A BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?net_treaty_off@@YAXHHHH@Z)(int, int, int, int)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055B760 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_treaty_off(int faction_id, int faction_id_with, int treaty,
                            int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTREATYOFF, faction_id, faction_id_with, treaty);
        message_data(0x2442, 0, faction_id, faction_id_with, treaty, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x442);
        }
    } else {
        treaty_off(faction_id, faction_id_with, treaty);
    }
}
/*
Purpose: net_treaty_on's five-argument form - set or clear one treaty bit in
         one move, so the caller does not have to know which direction it is.
// ORIGINAL: 0x005392A0 ?net_set_treaty@@YAXHHHHH@Z 0x005392A0-0x00539310 BYTE_EXACT
// size      112 bytes
// prototype void (__cdecl ?net_set_treaty@@YAXHHHHH@Z)(int, int, int, int, int)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055BB30 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_set_treaty(int faction_id, int faction_id_with, int treaty,
                            int set, int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingSETTREATY, faction_id, faction_id_with, treaty);
        message_data(0x2443, 0, faction_id, faction_id_with, treaty, set);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x443);
        }
    } else {
        set_treaty(faction_id, faction_id_with, treaty, set);
    }
}
/*
Purpose: net_treaty_on's agenda counterpart.
// ORIGINAL: 0x00539310 ?net_agenda_on@@YAXHHHH@Z 0x00539310-0x0053937A BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?net_agenda_on@@YAXHHHH@Z)(int, int, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055BA80 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_agenda_on(int faction_id, int faction_id_with, int agenda,
                           int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingAGENDAON, faction_id, faction_id_with, agenda);
        message_data(0x2444, 0, faction_id, faction_id_with, agenda, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x444);
        }
    } else {
        agenda_on(faction_id, faction_id_with, agenda);
    }
}
/*
Purpose: net_agenda_on's mirror.
// ORIGINAL: 0x00539380 ?net_agenda_off@@YAXHHHH@Z 0x00539380-0x005393EA BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?net_agenda_off@@YAXHHHH@Z)(int, int, int, int)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055B820 0x00592EE0 0x006262F0
// LEVER: the archive transcription had an early `return` inside the
//        `wait != 0` branch. The image has none - it falls through to the
//        shared pop/je tail every other net_* body ends with, and the extra
//        jump moves the whole tail. Written like net_agenda_on it matches.
Status: Complete
*/
void __cdecl net_agenda_off(int faction_id, int faction_id_with, int agenda,
                            int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingAGENDAOFF, faction_id, faction_id_with, agenda);
        message_data(0x2445, 0, faction_id, faction_id_with, agenda, 0);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x445);
        }
    } else {
        agenda_off(faction_id, faction_id_with, agenda);
    }
}
/*
Purpose: net_set_treaty's agenda counterpart.
// ORIGINAL: 0x005393F0 ?net_set_agenda@@YAXHHHHH@Z 0x005393F0-0x00539460 BYTE_EXACT
// size      112 bytes
// prototype void (__cdecl ?net_set_agenda@@YAXHHHHH@Z)(int, int, int, int, int)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x0055BBA0 0x00592EE0 0x006262F0
Status: Complete
*/
void __cdecl net_set_agenda(int faction_id, int faction_id_with, int agenda,
                            int set, int wait) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingSETAGENDA, faction_id, faction_id_with, agenda);
        message_data(0x2446, 0, faction_id, faction_id_with, agenda, set);
        if (wait != 0) {
            NetDaemonNet.await_diplo(0x446);
        }
    } else {
        set_agenda(faction_id, faction_id_with, agenda, set);
    }
}

