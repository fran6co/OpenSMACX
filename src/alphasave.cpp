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
#include "alphasave.h"

// Image references from AlphaSave::show/save/load store this, hide clears it,
// and filebox_draw tests it. The storage is distinct from ConsoleGlobal.
// RECOVERY_OBJECT: v2 0x006A7524 0x006A7528 ?ActiveAlphaSave@@3PAVAlphaSave@@A 0x00407BE0
AlphaSave *ActiveAlphaSave = 0;

void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll);

// ORIGINAL: 0x00407350 ??0AlphaSave@@QAE@XZ 0x00407350-0x00407410;0x00650BF0-0x00650C60
AlphaSave::AlphaSave() {
}

// ORIGINAL: 0x00408170 ??1AlphaSave@@QAE@XZ 0x00408170-0x00408383;0x00650D10-0x00650E20
// symbol    ??1AlphaSave@@UAE@XZ
// TRIED: plain reinterpret_cast pointer-offset calls into close()/dtors in disasm order; no local subobjects so no SEH frame is generated (diverges at #1, the missing try/finally prologue)
AlphaSave::~AlphaSave() {
    // The owner closes its own window before member destruction begins.
    GraphicWin::close();
}

// ORIGINAL: 0x00407910 ?close@AlphaSave@@QAEXXZ 0x00407910-0x00407915 BYTE_EXACT
// LEVER: thiscall shim - a synthetic GraphicWin class exposes close() as a member so reinterpret_cast<GraphicWin*>(this)->close() folds into the original's bare 5-byte jmp (same shim shape as 0x004210F0)
void AlphaSave::close() {
    GraphicWin::close();
}

// ORIGINAL: 0x00407CF0 ?on_double_clicked@AlphaSave@@QAEXH@Z 0x00407CF0-0x00407D06 BYTE_EXACT
void AlphaSave::on_double_clicked(int button_id) {
    file_win_.on_double_clicked(button_id);
}

// ORIGINAL: 0x00407ED0 ?on_listbox_scroll_delete@AlphaSave@@QAEHPAUScroll@@@Z 0x00407ED0-0x00407EF3 BYTE_EXACT
// symbol    ?on_listbox_scroll_delete@AlphaSave@@QAEHPAVScroll@@@Z
int AlphaSave::on_listbox_scroll_delete(Scroll *scroll) {
    if (Win::is_visible()) {
        scroll_hide(this, scroll);
    }
    return 0;
}
