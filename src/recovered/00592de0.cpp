// ORIGINAL: 0x00592DE0 sub_592de0 0x00592DE0-0x00592E0E BYTE_EXACT
// size      46 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532940
// 0x00592DE0  sub_592de0  ->  _sub_592de0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern int in_flight_sends;

void __cdecl sub_592de0(char *a1, unsigned long a2, int a3) {
    in_flight_sends++;
    ((NetDaemon *)g_0093cd90)->send_message(a1, a2, a3);
    in_flight_sends--;
}
