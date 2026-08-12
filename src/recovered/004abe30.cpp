// ORIGINAL: 0x004ABE30 BYTE_EXACT
// name      ?close_security@ReportIf@@QAEXXZ
// size      50 bytes
// spans     0x004ABE30-0x004ABE62
// prototype void (__thiscall ?close_security@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004ABE3F 0x004ABE4E 0x004ABE5D
// 0x004ABE30  ?close_security@ReportIf@@QAEXXZ  ->  ?close_security@ReportIf@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void ReportIf::close_security() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<VCall *>(self + 0x1bf9c)->slot002();
    reinterpret_cast<VCall *>(self + 0x1d634)->slot002();
    reinterpret_cast<VCall *>(self + 0x1cae8)->slot002();
}
