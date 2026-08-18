// ORIGINAL: 0x00407ED0 ?on_listbox_scroll_delete@AlphaSave@@QAEHPAUScroll@@@Z 0x00407ED0-0x00407EF3 BYTE_EXACT
// size      35 bytes
// prototype int (__thiscall ?on_listbox_scroll_delete@AlphaSave@@QAEHPAUScroll@@@Z)(AlphaSave* this, Scroll*)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045E540 0x005F7E90
// 0x00407ED0  ?on_listbox_scroll_delete@AlphaSave@@QAEHPAUScroll@@@Z  ->  ?on_listbox_scroll_delete@AlphaSave@@QAEHPAUScroll@@@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int AlphaSave::on_listbox_scroll_delete(Scroll* a1) {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        scroll_hide(reinterpret_cast<GraphicWin *>(this), a1);
    }
    return 0;
}
