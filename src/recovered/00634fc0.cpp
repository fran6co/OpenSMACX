// ORIGINAL: 0x00634FC0 BYTE_EXACT
// 0x00634FC0  ?UNK1@TexHeap@@QAEHH@Z  ->  ?UNK1@TexHeap@@QAEHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int TexHeap::UNK1(int a1) {
    UNK2(a1);
    return reinterpret_cast<int>(this);
}
