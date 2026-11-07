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
