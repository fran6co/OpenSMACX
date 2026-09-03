// ORIGINAL: 0x004E09E0 ?editor_save@Console@@QAEXXZ 0x004E09E0-0x004E09F9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?editor_save@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ABB90
// 0x004E09E0  ?editor_save@Console@@QAEXXZ  ->  ?editor_save@Console@@QAEXXZ
//
// BLOCKED (2026-09-03, P1 calibration) on infowin.h, which this TU does not
// own. 0x007AD330 is INSIDE the process-wide InfoWin at 0x007AD2A0 that the
// tree already binds as console.h's ConsoleInfoWin - it is ConsoleInfoWin's
// field at +0x90 (`field_90_`), and InfoWin's field block is PRIVATE. The
// image stores `mov dword ptr [0x7ad330], 7` from Console. The remedy is
// infowin.h's to make: either name that member protected/public (access
// changes no offset - the same argument buffer.h's 0x4.. block documents) or
// give InfoWin a setter; there is no friend to lean on, and `InfoWin::change`
// is a three-byte `ret 4`, so it is not the setter the scaffold hoped for.
//
// The rest of the body is settled: save_map, then ConsoleInfoWin.on_redraw(),
// which the image reaches with `mov ecx, 0x7ad2a0; jmp 0x458910` - a tail
// call, so the C++ is a plain `return ConsoleInfoWin.on_redraw();` shape.
//
// `save_map` (0x005ABF20) is also undeclared; it is the one remaining free
// function this body needs.

class InfoWin { public:
    void on_redraw();
};

void Console::editor_save() {
    save_map();
    *g_007ad330 = 7;
    reinterpret_cast<InfoWin *>(g_007ad2a0)->on_redraw();
}
