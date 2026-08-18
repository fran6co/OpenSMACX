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
#include "dialogs.h"

/*
Purpose: Close the dialog manager; the legacy implementation is intentionally empty.
// ORIGINAL: 0x00612960 ?close@Dialogs@@QAEXXZ 0x00612960-0x00612961 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?close@Dialogs@@QAEXXZ)(Dialogs* this)
// callers   20   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Dialogs::close() {
}

void __fastcall dialogs_close_redirect(Dialogs *self, void *) {
    self->close();
}
