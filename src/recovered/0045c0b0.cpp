// ORIGINAL: 0x0045C0B0 BYTE_EXACT
// 0x0045C0B0  ?chat@MainInterface@@QAEXPADH@Z  ->  ?chat@MainInterface@@QAEXPADH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MainInterface::chat(char *a1, int a2) {
    reinterpret_cast<StringBox *>(reinterpret_cast<char *>(this) + 0x1EACC)->add(a1, 0x2710, 2);
    reinterpret_cast<MessageWin *>(g_007f67f8)->add_message(a1, g_0068fa30[a2]);
}
