// 0x005F93A0  ?hide@PullDown@@QAEXXZ  ->  ?hide@PullDown@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void PullDown::hide() {
    reinterpret_cast<VCall *>(this)->slot058();
    reinterpret_cast<Win *>(this)->hide();
    do_all_draws();
}
