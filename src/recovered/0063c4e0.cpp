// ORIGINAL: 0x0063C4E0 BYTE_EXACT
// name      sub_63c4e0
// size      22 bytes
// spans     0x0063C4E0-0x0063C4F6
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0063C4E0  sub_63c4e0  ->  _sub_63c4e0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Run the teardown step for the given instance, but only while it is
         still the current one.
Original Offset: 0063C4E0
Return Value: n/a
Status: Complete
*/
// Catalogued as a nullary `int` function; the disassembly reads one stack
// argument at [esp+4] and never sets eax.
//
// The callee is __THISCALL. Forwarding `a1` as an ordinary cdecl argument
// puts it in eax and tops out at MNEMONIC_ONLY - all seven mnemonics
// agreeing, one register wrong - across eight source shapes. Only phrasing
// the tail call as a member on `a1` reproduces ecx.
struct S_0063C7C0 { void TailMethod(); };

extern "C" void __cdecl sub_63c4e0(int a1) {
    if (a1 != 0 && *g_009b7ae0 == a1) {
        reinterpret_cast<S_0063C7C0 *>(a1)->TailMethod();
    }
}
