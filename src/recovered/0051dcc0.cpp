// ORIGINAL: 0x0051DCC0 BYTE_EXACT
// name      sub_51dcc0
// size      54 bytes
// spans     0x0051DCC0-0x0051DCF6
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x0051DCC0  sub_51dcc0  ->  _sub_51dcc0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern int DAT_009bc06c;

extern "C" void __cdecl sub_51dcc0(int a1, int a2) {
    if (a2) {
        DAT_009bc06c |= (1 << a1);
    } else {
        int v = DAT_009bc06c;
        v &= ~(1 << a1);
        DAT_009bc06c = v;
    }
}
