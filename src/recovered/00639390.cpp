// 0x00639390  sub_639390  ->  _sub_639390
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Copy the caller's string into the shared buffer.
Original Offset: 00639390
Return Value: n/a
Status: Complete
*/
// The `repne scasb` + `rep movsd/movsb` sequence is not hand-rolled: it is
// VC6's own /Oi expansion of a `strcpy` call, so calling strcpy and
// discarding the result reproduces it exactly. Writing the loop by hand gives
// the wrong shape.
//
// Catalogued as nullary returning int, and both are wrong: the body reads a
// real stack argument, and `return 0;` inserts an `xor eax,eax` before the
// `and ecx,3`.
extern "C" char *strcpy(char *, const char *);

extern "C" void __cdecl sub_639390(char *source) {
    strcpy(reinterpret_cast<char *>(g_009c0d60), source);
}
