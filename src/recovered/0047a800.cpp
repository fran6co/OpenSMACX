// ORIGINAL: 0x0047A800 BYTE_EXACT
// 0x0047A800  ?timer_daemon@NetMsg@@QAAXH@Z  ->  ?timer_daemon@NetMsg@@QAAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Signature change: [ebp+8] is read directly (compared to 0x46) and
// [ebp+0xC] is never touched. A real `NetMsg::timer_daemon(int a1)`
// out-of-line __cdecl member definition puts `this` at [ebp+8] and `a1`
// at [ebp+0xC] (confirmed by compiling that exact form and disassembling
// it), which is backwards from what this body needs - so this is written
// as the equivalent free function with the sole meaningful argument in
// the position the callee actually reads, dropping the always-dead
// receiver instead of threading it through for no byte benefit.
void __cdecl fn_0047a800(int a1) {
    if (a1 - 0x46 != 0) {
        if (*g_00805304 != 0) {
            *g_00805304 = 0;
            reinterpret_cast<Time *>(g_0080530c)->close();
            reinterpret_cast<Popup *>(g_007fff80)->hide();
        }
    } else {
        if (*g_0080a6bc != 0) {
            *g_0080a6bc = 0;
            reinterpret_cast<Time *>(g_0080a6c4)->close();
            reinterpret_cast<Popup *>(g_00805338)->hide();
        }
    }
}
