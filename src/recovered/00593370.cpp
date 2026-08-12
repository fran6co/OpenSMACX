// ORIGINAL: 0x00593370 BYTE_EXACT
// name      sub_593370
// size      28 bytes
// spans     0x00593370-0x0059338C
// prototype 
// callers   0   call targets   1
// 0x00593370  sub_593370  ->  _sub_593370
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl sub_593370() {
    reinterpret_cast<NetDaemon *>(g_0093cd90)->synch(1, 0, 0, 0, 0, 1, 0x2101);
}
