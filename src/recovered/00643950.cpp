// ORIGINAL: 0x00643950 BYTE_EXACT
// name      sub_643950
// size      49 bytes
// spans     0x00643950-0x00643981
// prototype 
// callers   1   call targets   1
// 0x00643950  sub_643950  ->  _sub_643950
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" int __cdecl sub_643950(int a1, int a2, int a3) {
    typedef int(__cdecl * Sub642b10)(int, int, int);
    ((Sub642b10)sub_642b10)(a1, a2, a3);
    typedef void(__cdecl * Callback)(int, int);
    (*(Callback *)(a2 + 0x24))(*(int *)(a2 + 0x28), *(int *)(a1 + 0x24));
    (*(Callback *)(a2 + 0x24))(*(int *)(a2 + 0x28), a1);
    return 0;
}
