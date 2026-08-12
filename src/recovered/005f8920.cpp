// ORIGINAL: 0x005F8920 BYTE_EXACT
// name      ?init@PullDown@@QAEXPAUMenu@@@Z
// size      71 bytes
// spans     0x005F8920-0x005F8967
// prototype void (__thiscall ?init@PullDown@@QAEXPAUMenu@@@Z)(PullDown* this, Menu*)
// callers   12   call targets   1
// 0x005F8920  ?init@PullDown@@QAEXPAUMenu@@@Z  ->  ?init@PullDown@@QAEXPAUMenu@@@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void PullDown::init(Menu * a1) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    if (*g_009b7b68 == 0) {
        *g_009b7b68 = *g_009bb484;
    }
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<Menu **>(self + 0xf30) = a1;
    reinterpret_cast<GraphicWin *>(this)->init(0, 0, 0, 0, (char *)0, 0,
                                               (Win *)0, (Menu *)0,
                                               (BorderSizing *)0);
    *reinterpret_cast<int *>(self + 0x964) = 0;
}
