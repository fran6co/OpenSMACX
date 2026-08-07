// 0x0044CB40  sub_44cb40  ->  _sub_44cb40
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Set the current value, treating a negative argument as "keep what is
         already there".
Original Offset: 0044CB40
Return Value: the value now held
Status: Complete
*/
extern "C" int __cdecl sub_44cb40(int a1) {
    if (a1 < 0) {
        a1 = *g_007591ac;
    }
    *g_007591ac = a1;
    return a1;
}
