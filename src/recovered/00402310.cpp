// ORIGINAL: 0x00402310 BYTE_EXACT
// 0x00402310  sub_402310  ->  _sub_402310
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Publish the field at +4 to a global, then reset slot 0 to a fixed
         value - the shape of a detach step.
Original Offset: 00402310
Return Value: n/a
Status: Complete
*/
// Catalogued as `extern "C" int __cdecl sub_402310()`, which the disassembly
// contradicts: the receiver arrives in ecx, nothing touches the stack, and
// `ret` pops nothing. It is a nullary __thiscall member.
struct S_00402310 {
    int *field0;
    int field4;
    void Method();
};

void S_00402310::Method() {
    // Read BEFORE slot 0 is overwritten; that order is visible in the bytes.
    const int value = field4;
    field0 = g_006693ac;
    *g_009b3374 = value;
}
