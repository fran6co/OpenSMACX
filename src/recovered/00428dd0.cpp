// ORIGINAL: 0x00428DD0 BYTE_EXACT
// name      ?timer_callback_daemon@Credits@@QAAXH@Z
// size      23 bytes
// spans     0x00428DD0-0x00428DE7
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00428C00
// 0x00428DD0  ?timer_callback_daemon@Credits@@QAAXH@Z  ->  ?timer_callback_daemon@Credits@@QAAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl Credits::timer_callback_daemon(int a1) {
    if (this) {
        field_a14_++;
        draw_credits();
    }
}
