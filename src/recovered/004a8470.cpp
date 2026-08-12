// ORIGINAL: 0x004A8470 BYTE_EXACT
// name      ?close_labs@ReportIf@@QAEXXZ
// size      46 bytes
// spans     0x004A8470-0x004A849E
// prototype void (__thiscall ?close_labs@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609F20
// 0x004A8470  ?close_labs@ReportIf@@QAEXXZ  ->  ?close_labs@ReportIf@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void ReportIf::close_labs() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<ListBox *>(self + 0xa2d0)->close();
    reinterpret_cast<VCall *>(self + 0xba0c)->slot002();
    reinterpret_cast<VCall *>(self + 0xc558)->slot002();
}
