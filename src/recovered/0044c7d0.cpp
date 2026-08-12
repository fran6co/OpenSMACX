// ORIGINAL: 0x0044C7D0 BYTE_EXACT
// 0x0044C7D0  ?help_info_pop@@YAXPAD@Z  ->  ?help_info_pop@@YAXPAD@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl help_info_pop(int8* a1) {
    reinterpret_cast<BasePop *>(*g_007591b0)->string(a1);
}
