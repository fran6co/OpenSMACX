// ORIGINAL: 0x00444D90 ?diplo@@YAXHH@Z 0x00444D90-0x00444DA7 BYTE_EXACT
// TRIED: homed into game.cpp (its real TU) 2026-09-03 - does not compile there: the body's only call is `DiploWin::exec` at 0x00443F10, which diplowin.h does not declare and no product TU defines yet. Declaring `void exec(int,int)` on DiploWin without a definition is a link error, so the body stays here until 0x00443F10 is homed into diplowin.cpp and `exec` declared on the class. The `#include "diplowin.h"` game.cpp needs for it is already added.
// size      23 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00443F10
// 0x00444D90  ?diplo@@YAXHH@Z  ->  ?diplo@@YAXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

static int *const g_0073acd8 = (int *)0x0073ACD8;

void __cdecl diplo(int a1, int a2) {
    reinterpret_cast<DiploWin *>(g_0073acd8)->exec(a1, a2);
}
