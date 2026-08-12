// ORIGINAL: 0x0048BF20 BYTE_EXACT
// 0x0048BF20  ??_GPlanWin@@UAEPAXI@Z  ->  ??_GPlanWin@@UAEPAXI@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void* PlanWin::m_0048bf20(unsigned int a1) {
    char *base = reinterpret_cast<char *>(this) - 0x22050;
    reinterpret_cast<PlanWin *>(base + 0x22050)->~PlanWin();
    reinterpret_cast<GraphicWin *>(base + 0x22050)->~GraphicWin();
    if (a1 & 1) {
        ::operator delete(base);
    }
    return base;
}
