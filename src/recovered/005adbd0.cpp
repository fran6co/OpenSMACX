// 0x005ADBD0  ?timer_callback_daemon@ReplayWin@@QAAXH@Z  ->  ?timer_callback_daemon@ReplayWin@@QAAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The scaffolded signature used the reserved word `this` as a parameter
// name and arity 2; the disassembly reads exactly one argument at
// [ebp+8], so this corrects both.
void __cdecl fn_005adbd0(ReplayWin* a1) {
    if (a1) {
        a1->timer_callback();
    }
}
