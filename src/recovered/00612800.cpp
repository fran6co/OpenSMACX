// ORIGINAL: 0x00612800 BYTE_EXACT
// name      sub_612800
// size      14 bytes
// spans     0x00612800-0x0061280E
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x00612800  sub_612800  ->  _sub_612800
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: An MSVC vtordisp adjustor thunk: re-base `this` by the runtime
         displacement stored at -4 and a fixed 0xA18, then tail-call.
Original Offset: 00612800
Return Value: n/a
Status: Complete
*/
// Compiler-SYNTHESISED in the original, but it reproduces byte-exact as
// ordinary pointer arithmetic in a __thiscall member - no virtual-inheritance
// hierarchy has to be modelled. That matters beyond this row: the same shape
// is the whole `adjustor_thunks` cluster.
struct S_00612710 { void TailMethod(); };
struct S_00612800 { void Method(); };

void S_00612800::Method() {
    char *base = reinterpret_cast<char *>(this);
    base -= *reinterpret_cast<int *>(base - 4);
    base -= 0xA18;
    reinterpret_cast<S_00612710 *>(base)->TailMethod();
}
