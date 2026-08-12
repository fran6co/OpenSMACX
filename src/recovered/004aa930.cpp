// ORIGINAL: 0x004AA930 BYTE_EXACT
// 0x004AA930  ?close_projects@ReportIf@@QAEXXZ  ->  ?close_projects@ReportIf@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void ReportIf::close_projects() {
    char *self = reinterpret_cast<char *>(this);
    int state = *reinterpret_cast<int *>(self + 8);
    if (state != 4 && state != 5) {
        reinterpret_cast<WorldWin *>(g_008e9f60)->hide_all();
        VCall *entry = reinterpret_cast<VCall *>(g_007b39e8);
        do {
            entry->slot002();
            entry = reinterpret_cast<VCall *>(
                reinterpret_cast<char *>(entry) + 0xb4c);
        } while (reinterpret_cast<int>(entry) <=
                 reinterpret_cast<int>(g_007b4534));
    }
}
