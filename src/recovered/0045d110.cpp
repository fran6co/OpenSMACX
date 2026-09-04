// ORIGINAL: 0x0045D110 ?on_iface_mouse_leave@MainInterface@@QAEXHH@Z 0x0045D110-0x0045D13F BYTE_EXACT
// BLOCKED 2026-09-04 on the RECEIVER MAP, not on the bindings. The image is
//         mov esi,ecx / mov ecx,0x8c5568 / call reset_active /
//         cmp [esi+0x20cd8],1 / jne / lea ecx,[esi-0xa14] /
//         mov [esi+0x20cd8],0 / call redraw_complete. 0x008C5568 is console.h's
//         ConsoleStatusWin, so that half is named - but `lea ecx, [esi-0xa14]`
//         hands redraw_complete a receiver 0xA14 BELOW this, and 0x20CD8 is a
//         member the class map does not name. A negative adjustment means the
//         catalogued MainInterface is an INNER object: the real object starts
//         0xA14 bytes earlier and this method's `this` is not its front. That
//         is the same unresolved receiver map that defers MainInterface::init,
//         and it has to be settled first - a body homed against the wrong
//         receiver would compile and silently write to the wrong offset.
// size      47 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045C3A0 0x004B6530
// 0x0045D110  ?on_iface_mouse_leave@MainInterface@@QAEXHH@Z  ->  ?on_iface_mouse_leave@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MainInterface::on_iface_mouse_leave(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<StatusWin *>(g_008c5568)->reset_active();
    if (*reinterpret_cast<int *>(self + 0x20cd8) == 1) {
        *reinterpret_cast<int *>(self + 0x20cd8) = 0;
        reinterpret_cast<MainInterface *>(self - 0xa14)->
            redraw_complete();
    }
}
