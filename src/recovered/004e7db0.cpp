// 0x004E7DB0  sub_4e7db0  ->  _sub_4e7db0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Original Offset: 004E7DB0
Return Value: zero
Status: Complete
*/
extern "C" int __cdecl sub_4e7db0() {
    *reinterpret_cast<int *>(0x0090E998) = 0;
    return 0;
}
