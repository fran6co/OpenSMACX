// ORIGINAL: 0x005EFD00 sub_5efd00 0x005EFD00-0x005EFD1B BYTE_EXACT
// size      27 bytes
// prototype 
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EFD09 0x005EFD12
// 0x005EFD00  sub_5efd00  ->  _sub_5efd00
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__stdcall *StdFn)(int);

int __cdecl sub_5efd00() {
    StdFn fn = *reinterpret_cast<StdFn *>(g_00669334);
    *g_009b7b1c = fn(0);
    return (*g_009b7b20 = fn(1));
}
