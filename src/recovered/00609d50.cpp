// 0x00609D50  ?set_def_dialog_text_color@Dialog@@QAAXHHHHH@Z  ->  ?set_def_dialog_text_color@Dialog@@QAAXHHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// SIGNATURE CHANGE (proposal): the scaffold's default guess is a
// non-static __cdecl member, which pushes `this` as a hidden first stack
// argument and shifts every real parameter 4 bytes high - measured: every
// `[esp+N]` read compiled 4 too high, and the four store constants (the
// only non-relocated part of this body) diverge nowhere else
// (SHAPE_EXACT with the class member; BYTE_EXACT once `this` is dropped).
// `this` is never read anywhere in the body (no `[ecx+N]` access), so it
// is not a member at all - defined here as a plain extern "C" function
// instead, matching the BaseWin::timer_callback precedent for a
// same-shaped mismatch (src/basewin.cpp:277). The class's own declaration
// is left as-is (unused, never defined, so it emits no code and does not
// collide with the single required external symbol below).
//
// The four global arrays are 0xC (not 0x10) apart, so index 3 of one
// overlaps index 0 of the next - `extern int g[]` (no fixed address) is
// used rather than the `static int *const` spelling so the indexed store
// keeps its `[eax*4 + ADDR]` form instead of folding to `[eax*4]` plus a
// separate base add (EXTERN-SYMBOL LEVER).
extern int g_6970ac[];
extern int g_6970b8[];
extern int g_6970c4[];
extern int g_6970d0[];

extern "C" void __cdecl dialog_set_def_dialog_text_color(int a1, int a2, int a3, int a4, int a5) {
    if (a5 < 0 || a5 > 3) {
        a5 = 0;
    }
    g_6970ac[a5] = a1;
    g_6970b8[a5] = a2;
    g_6970c4[a5] = a3;
    g_6970d0[a5] = a4;
}
