// 0x004E09E0  ?editor_save@Console@@QAEXXZ  ->  ?editor_save@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class InfoWin { public:
    void on_redraw();
};

void Console::editor_save() {
    save_map();
    *g_007ad330 = 7;
    reinterpret_cast<InfoWin *>(g_007ad2a0)->on_redraw();
}
