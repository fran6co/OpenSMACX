// ORIGINAL: 0x004B8830 BYTE_EXACT
// name      ?draw@StatusWin@@QAEXHHHHH@Z
// size      90 bytes
// spans     0x004B8830-0x004B888A
// prototype void (__thiscall ?draw@StatusWin@@QAEXHHHHH@Z)(StatusWin* this, int, int, int, int, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B6570 0x005D5930
// 0x004B8830  ?draw@StatusWin@@QAEXHHHHH@Z  ->  ?draw@StatusWin@@QAEXHHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- the global at 0x669328 holds a plain load of a function pointer;
// call through it with the right (stdcall) convention so the callee pops. ----
typedef int (__stdcall *UnionRectProc)(RECT*, const RECT*, const RECT*);

void StatusWin::draw(int a1, int a2, int a3, int a4, int a5) {
    RECT local14;
    draw_status(a1, a2, a3, a4, a5);
    UnionRectProc pUnionRect = reinterpret_cast<UnionRectProc>(*g_00669328);
    pUnionRect(&local14, reinterpret_cast<RECT*>(g_007af50c),
               reinterpret_cast<RECT*>(g_007af4ec));
    pUnionRect(&local14, &local14, reinterpret_cast<RECT*>(g_007af4fc));
    reinterpret_cast<GraphicWin*>(g_007ae820)->soft_update(&local14);
}
