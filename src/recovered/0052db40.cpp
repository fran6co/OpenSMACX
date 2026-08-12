// ORIGINAL: 0x0052DB40 BYTE_EXACT
// name      sub_52db40
// size      39 bytes
// spans     0x0052DB40-0x0052DB67
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0052DB40  sub_52db40  ->  _sub_52db40
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void sub_52db40() {
    char *record = reinterpret_cast<char *>(g_0093e99e) - 2;
    for (int i = 8; i != 0; --i) {
        record[0] = -1;
        record[1] = 0;
        record[2] = 0;
        record[3] = -1;
        record[4] = 2;
        record += 0x19c;
    }
}
