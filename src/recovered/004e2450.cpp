// 0x004E2450  sub_4e2450  ->  _sub_4e2450
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Original Offset: 004E2450
Return Value: n/a
Status: Complete
*/
extern "C" void __cdecl sub_4e2450() {
    *reinterpret_cast<int *>(0x0090E778) = 0;
    *reinterpret_cast<char *>(0x0090E79C) = 0;
}
