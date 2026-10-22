/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "netdaemon.h"
#include "game.h"

// Application-facing network notifications and the network-aware wait bridge.
// The daemon implementation and all state storage remain in their real owners.

// ORIGINAL: 0x0051F2C0 ?wait_2@@YAXXZ 0x0051F2C0-0x0051F2D8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bb3b3233842b050fb0840e1cc6c226e8f30f7055:src/recovered/0051f2c0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "netdaemon.cpp / netdaemon.h: use existing NetDaemonNet.net_tasks(), IsMultiplayerNet and wait_task(). Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
void __cdecl wait_2() {
    if (IsMultiplayerNet != 0) {
        NetDaemonNet.net_tasks();
    } else {
        wait_task();
    }
}

/*
Purpose: Announce a vehicle's synched state to the network. All four "no
         text, one repeated flag" parameters are fixed by the caller, so only
         the opcode and the id vary across this family.
// ORIGINAL: 0x00593220 ?synch_veh@@YAXH@Z 0x00593220-0x00593242 BYTE_EXACT
// size      34 bytes
// prototype void (__cdecl ?synch_veh@@YAXH@Z)(int vehID)
// callers   32   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_veh(int id) {
    NetDaemonNet.synch(0x11, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a base's synched state to the network.
// ORIGINAL: 0x00593250 ?synch_base@@YAXH@Z 0x00593250-0x00593272 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   6   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_base(int id) {
    NetDaemonNet.synch(0x13, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's energy synched state to the network.
// ORIGINAL: 0x00593280 ?synch_energy@@YAXH@Z 0x00593280-0x005932A2 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_energy(int id) {
    NetDaemonNet.synch(0xB, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's current research target to the network.
// ORIGINAL: 0x005932B0 ?synch_researching@@YAXH@Z 0x005932B0-0x005932D2 BYTE_EXACT
// size      34 bytes
// prototype void (__cdecl ?synch_researching@@YAXH@Z)(int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_researching(int id) {
    NetDaemonNet.synch(0xA, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's leader to the network.
// ORIGINAL: 0x005932E0 ?synch_leader@@YAXH@Z 0x005932E0-0x00593302 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_leader(int id) {
    NetDaemonNet.synch(0x6, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's AI state to the network.
// ORIGINAL: 0x00593310 ?synch_ai@@YAXH@Z 0x00593310-0x00593332 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_ai(int id) {
    NetDaemonNet.synch(0x8, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's completed research to the network.
// ORIGINAL: 0x00593340 ?synch_research@@YAXH@Z 0x00593340-0x00593362 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_research(int id) {
    NetDaemonNet.synch(0x9, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's social/resource allocation state to the
         network.
// ORIGINAL: 0x00593390 ?synch_alloc@@YAXH@Z 0x00593390-0x005933B2 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_alloc(int id) {
    NetDaemonNet.synch(0xC, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's social engineering state to the network.
// ORIGINAL: 0x005933C0 ?synch_soc@@YAXH@Z 0x005933C0-0x005933E2 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_soc(int id) {
    NetDaemonNet.synch(0xD, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's protocol/pact state to the network.
// ORIGINAL: 0x005933F0 ?synch_proto@@YAXH@Z 0x005933F0-0x00593412 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_proto(int id) {
    NetDaemonNet.synch(0xE, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce an observer's state to the network.
// ORIGINAL: 0x00593420 ?synch_obs@@YAXH@Z 0x00593420-0x00593442 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_obs(int id) {
    NetDaemonNet.synch(0x10, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a diplomatic state change between two factions to the
         network. Unlike the rest of this family, both leading arguments
         come from the caller instead of the second being a fixed zero.
// ORIGINAL: 0x00593450 ?synch_diplo@@YAXHH@Z 0x00593450-0x00593474 BYTE_EXACT
// size      36 bytes
// prototype void (__cdecl ?synch_diplo@@YAXHH@Z)(int factionID, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_diplo(int a, int b) {
    NetDaemonNet.synch(0x16, a, b, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's template/design state to the network.
// ORIGINAL: 0x005934B0 ?synch_template@@YAXH@Z 0x005934B0-0x005934D2 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_template(int id) {
    NetDaemonNet.synch(0x18, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's sensor/territory radius state to the network.
// ORIGINAL: 0x005934E0 ?synch_radius@@YAXH@Z 0x005934E0-0x00593502 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_radius(int id) {
    NetDaemonNet.synch(0x23, id, 0, 0, nullptr, 1, 0x2101);
}

