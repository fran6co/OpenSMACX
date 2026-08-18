// ORIGINAL: 0x00455E80 ?load_fixed_sprites@@YAXXZ 0x00455E80-0x00455EA8 BYTE_EXACT
// size      40 bytes
// prototype 
// callers   1   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00423570 0x00451030 0x00451A10 0x00454D10 0x00455610 0x00455E50
// 0x00455E80  ?load_fixed_sprites@@YAXXZ  ->  ?load_fixed_sprites@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl load_cycling();

void __cdecl load_fixed_sprites() {
    load_palette();
    load_misc();
    load_interface(0);
    load_voxels();
    load_deswin_sprites();
    colortables_init();
    load_cycling();
}
