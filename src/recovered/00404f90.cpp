// 0x00404F90  ?on_nc_hittest@Popup@@QAEHHH@Z  ->  ?on_nc_hittest@Popup@@QAEHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Popup::on_nc_hittest(int a1, int a2) {
    return reinterpret_cast<Win *>(this)->on_nc_hittest(a1, a2);
}
