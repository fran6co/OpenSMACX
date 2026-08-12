// ORIGINAL: 0x005CF120 BYTE_EXACT
// name      sub_5cf120
// size      36 bytes
// spans     0x005CF120-0x005CF144
// prototype 
// callers   1   call targets   0
// 0x005CF120  sub_5cf120  ->  _sub_5cf120
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Look up the message for an error code.
Original Offset: 005CF120
Return Value: the message, or the fallback when the code is unknown
Status: Complete
*/
// `__fastcall(int)`, not the catalogued nullary `__cdecl`: the argument
// arrives in ecx - `cmp ecx,[eax]` with no prior load - and `ret` pops
// nothing because a register-passed argument needs no cleanup.
extern "C" char *__fastcall sub_5cf120(int code) {
    // Top-tested `while` again; a do-while gets rotated. Signed compare on
    // the addresses, as in 0x0045D2E0.
    int index = 0;
    int *entry = g_00694e28;
    while (reinterpret_cast<int>(entry) < reinterpret_cast<int>(g_00695108)) {
        if (code == *entry) {
            return reinterpret_cast<char *>(g_00694e2c[index * 2]);
        }
        entry += 2;
        ++index;
    }
    return reinterpret_cast<char *>(g_00695b34);
}
