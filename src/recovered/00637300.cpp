// ORIGINAL: 0x00637300 Video_unk11 0x00637300-0x00637308 BYTE_EXACT
// symbol    ?Video_unk11@@YAHXZ
// size      8 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006371D0
// 0x00637300  Video_unk11  ->  _Video_unk11
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __stdcall Video_unk10(int);

int __cdecl Video_unk11() {
    return Video_unk10(0);
}
