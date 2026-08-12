// ORIGINAL: 0x0045C180 BYTE_EXACT
// name      ?timer_callback_daemon@MainInterface@@QAEXH@Z
// size      116 bytes
// spans     0x0045C180-0x0045C1F4
// prototype void (__thiscall ?timer_callback_daemon@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D5930 0x00616730
// 0x0045C180  ?timer_callback_daemon@MainInterface@@QAEXH@Z  ->  ?timer_callback_daemon@MainInterface@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__stdcall *UnionRectProc)(RECT *, const RECT *, const RECT *);

void MainInterface::timer_callback_daemon(int a1) {
    char *self = reinterpret_cast<char *>(this);
    RECT local;
    switch (a1) {
    case 0:
        reinterpret_cast<Time *>(self + 0x25040)->stop();
        break;
    case 1:
        if (*reinterpret_cast<int *>(self + 0x216f0) == -1) {
            reinterpret_cast<Time *>(self + 0x25068)->stop();
            return;
        }
        *reinterpret_cast<int *>(self + 0x216f0) += 1;
        reinterpret_cast<UnionRectProc>(*g_00669328)(
            &local,
            reinterpret_cast<RECT *>(self + 0xde0),
            reinterpret_cast<RECT *>(self + 0xdf0));
        reinterpret_cast<GraphicWin *>(this)->soft_update(&local);
        break;
    }
}
