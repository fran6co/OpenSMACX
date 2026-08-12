// ORIGINAL: 0x0051DDF0 BYTE_EXACT
// name      ??_GConsole@@UAEPAXI@Z
// size      54 bytes
// spans     0x0051DDF0-0x0051DE26
// prototype void* (__thiscall ??_GConsole@@UAEPAXI@Z)(Console* this, unsigned int)
// callers   0   call targets   3
// 0x0051DDF0  ??_GConsole@@UAEPAXI@Z  ->  ??_GConsole@@UAEPAXI@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef void (__cdecl *DeleteProc)(void *);

void* Console::m_0051ddf0(unsigned int a1) {
    char *base = reinterpret_cast<char *>(this) - 0x23d94;
    char *adjusted = base + 0x23d94;
    reinterpret_cast<Console *>(adjusted)->~Console();
    reinterpret_cast<GraphicWin *>(adjusted)->~GraphicWin();
    if (a1 & 1) {
        (reinterpret_cast<DeleteProc>(&fn_0064557f))(base);
    }
    return base;
}
