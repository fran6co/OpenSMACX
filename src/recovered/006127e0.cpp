// ORIGINAL: 0x006127E0
// 0x006127E0  sub_6127e0  ->  _sub_6127e0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: A this-adjusting thunk: re-base the receiver by the displacement at
         -0x448 and a fixed 0x444, then tail-call.
Original Offset: 006127E0
Return Value: whatever the adjusted callee returns
Status: Complete
*/
// Catalogued as `extern "C" int __cdecl sub_6127e0()`; the disassembly says
// otherwise - ecx is read and adjusted with no stack traffic at all.
struct S_00612710_Callee { int Method(); };
struct S_006127E0 { int Forward(); };

int S_006127E0::Forward() {
    char *base = reinterpret_cast<char *>(this);
    base -= *reinterpret_cast<int *>(base - 0x448);
    base -= 0x444;
    return reinterpret_cast<S_00612710_Callee *>(base)->Method();
}
