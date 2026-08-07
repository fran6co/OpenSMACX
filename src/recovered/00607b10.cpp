// 0x00607B10  ?timer_callback_daemon@BaseButton@@QAAXHH@Z  ->  ?timer_callback_daemon@BaseButton@@QAAXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The scaffolded signature used the reserved word `this` and arity 3;
// the disassembly is frameless and reads [esp+4] (the int) and [esp+8]
// (the object pointer) - two arguments, in that order.
void __cdecl fn_00607b10(int a1, BaseButton* a2) {
    if (a2) {
        a2->timer_callback(a1);
    }
}
