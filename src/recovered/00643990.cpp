// 0x00643990  sub_643990  ->  _sub_643990
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// PROPOSAL: sub_643990(int, int, int, int) - the given head has zero
// parameters, but four are read off the stack ([esp+8], [esp+8] again
// under a different name, [esp+0x14] twice at two different push
// depths) and `ret` pops nothing extra beyond what __cdecl already
// implies, so nothing pins them elsewhere.
//
// huft_build and sub_644100 are both catalogued here with 0 args
// (their own rows are unrecovered too), but this call needs 8 and 2
// respectively. Redeclaring the same extern "C" name with a different
// arity is a hard conflict (C2733), so each is invoked through a
// function-pointer reinterpret_cast off its address instead - same
// symbol and relocation, the arity this call site actually needs.
typedef int (__cdecl *HuftBuildFn)(int, int, int, int, int, int, int, int);
typedef int (__cdecl *Sub644100Fn)(int, int);

extern "C" int __cdecl sub_643990(int a1, int a2, int a3, int a4) {
    HuftBuildFn huft = reinterpret_cast<HuftBuildFn>(&huft_build);
    int r = huft(a1, 0x13, 0x13, 0, 0, a3, a2, a4);
    if (r == -3) {
        *reinterpret_cast<int *>(a4 + 0x18) = reinterpret_cast<int>(g_006a5c9c);
        return r;
    }
    if (r == -5) {
        Sub644100Fn fn2 = reinterpret_cast<Sub644100Fn>(&sub_644100);
        fn2(*reinterpret_cast<int *>(a3), a4);
        *reinterpret_cast<int *>(a4 + 0x18) = reinterpret_cast<int>(g_006a5c78);
        r = -3;
    }
    return r;
}
