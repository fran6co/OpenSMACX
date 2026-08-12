// ORIGINAL: 0x0043F5C0 BYTE_EXACT
// 0x0043F5C0  ?stop@DiploPop@@QAEXXZ  ->  ?stop@DiploPop@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DiploPop::stop() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<SubInterface *>(self + 0xa14)->release_iface_mode();
    reinterpret_cast<Flic *>(self + 0x15cc)->close();
    reinterpret_cast<Flic *>(self + 0xae8)->close();
    *reinterpret_cast<int *>(self + 0xa94) = -1;
}
