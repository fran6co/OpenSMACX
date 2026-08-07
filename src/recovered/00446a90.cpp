// 0x00446A90  ?stop@FX@@QAEXH@Z  ->  ?stop@FX@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void FX::stop(int a1) {
    reinterpret_cast<VCall *>(reinterpret_cast<char *>(this) + a1 * 0x6C)->slot008();
}
