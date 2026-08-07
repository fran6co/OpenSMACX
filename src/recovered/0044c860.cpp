// 0x0044C860  sub_44c860  ->  _sub_44c860
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Original Offset: 0044C860
Return Value: n/a
Status: Complete
*/
extern "C" void __cdecl sub_44c860(int value) {
    *reinterpret_cast<int *>(0x007591C4) = value;
}
