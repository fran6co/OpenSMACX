// ORIGINAL: 0x0063C390 BYTE_EXACT
// name      sub_63c390
// size      14 bytes
// spans     0x0063C390-0x0063C39E
// prototype 
// callers   0   call targets   0
// 0x0063C390  sub_63c390  ->  _sub_63c390
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Forward to the teardown step, but only for the current instance.
Original Offset: 0063C390
Return Value: n/a
Status: Complete
*/
// A nullary __thiscall member, not the catalogued `__cdecl sub_63c390()`:
// ecx is compared directly against a global and `ret` pops nothing.
struct S_0063c390 {
    void Method();
    void TailMethod();
};

void S_0063c390::Method() {
    if (*g_009b7ae0 == reinterpret_cast<int>(this)) {
        TailMethod();
    }
}
