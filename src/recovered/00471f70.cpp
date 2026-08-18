// ORIGINAL: 0x00471F70 ?listbox_fill@MessageWin@@QAAHPAUGraphicWin@@HHPAURECT@@@Z 0x00471F70-0x00471FC2 BYTE_EXACT
// size      82 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D5250 0x005E2DD3
// 0x00471F70  ?listbox_fill@MessageWin@@QAAHPAUGraphicWin@@HHPAURECT@@@Z  ->  ?listbox_fill@MessageWin@@QAAHPAUGraphicWin@@HHPAURECT@@@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl listbox_fill(GraphicWin * a1, int a2, int a3, RECT * a4) {
    RECT rect = *reinterpret_cast<RECT *>(reinterpret_cast<char *>(a1) + 0x474);
    a1->fill(9);
    reinterpret_cast<Buffer *>(reinterpret_cast<char *>(a1) + 0x444)
        ->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d7f8));
    return 1;
}
