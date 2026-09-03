// ORIGINAL: 0x004D0450 ?skip@Console@@QAEXH@Z 0x004D0450-0x004D0494 BYTE_EXACT
// size      68 bytes
// prototype void (__thiscall ?skip@Console@@QAEXH@Z)(Console* this, int vehID)
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052DC70 0x00531780 0x005C1B70
// 0x004D0450  ?skip@Console@@QAEXH@Z  ->  ?skip@Console@@QAEXH@Z
//
// BLOCKED (2026-09-03, P1 calibration) on netdaemon.h, which this TU does not
// own: 0x00531780 is a NetDaemon member called on 0x0093CD90
// (netdaemon.cpp's NetDaemonNet) with (veh_id, -1, 0) - the catalogue gives it
// no name. It needs a declaration on NetDaemon and a pending_bodies forwarder.
//
// The other two callees ARE settled: `veh_ready` is the free
// ?veh_ready@@YAHH@Z at 0x005C1B70 (src/unrecovered/005c1b70.cpp), whose
// forwarder shape is already written in pending_bodies.cpp, and `not_my_turn`
// is game.h:312's MEASURED inline - note the image CALLS 0x0052DC70 here
// rather than inlining it, so if the inline expansion shows up in the compiled
// body, the spelling has to go back to a real call. The Console store at
// 0x23BE4 also needs naming in console.h before homing.

void Console::skip(int a1) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    if (veh_ready(a1) != 0) {
        int result = not_my_turn();
        if (result == 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->order_veh(a1, -1, result);
            return;
        }
    }
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0x23be4) = 0;
}
