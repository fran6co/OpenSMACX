// ORIGINAL: 0x00559140 BYTE_EXACT
// name      ?get_adjective_insult_faction@@YAPADH@Z
// size      26 bytes
// spans     0x00559140-0x0055915A
// prototype int8* (__cdecl ?get_adjective_insult_faction@@YAPADH@Z)(int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x00559140  ?get_adjective_insult_faction@@YAPADH@Z  ->  ?get_adjective_insult_faction@@YAPADH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Address the insult adjective row for a faction.
Original Offset: 00559140
Return Value: the row's text
Status: Complete
*/
char *__cdecl get_adjective_insult_faction(int a1) {
    // `g_00946c34` is scaffolded as `int *const`, so its own *4 scale supplies
    // the final `lea`; the 359 stride is what VC6 decomposes into the
    // lea/lea/shl/sub chain.
    return reinterpret_cast<char *>(g_00946c34 + a1 * 359);
}
