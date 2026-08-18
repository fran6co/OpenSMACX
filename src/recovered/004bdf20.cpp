// ORIGINAL: 0x004BDF20 ?terrain_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z 0x004BDF20-0x004BDF66 BYTE_EXACT
// size      70 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004632D0
// 0x004BDF20  ?terrain_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z  ->  ?terrain_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The emitted MapWin::gen_terrain_poly above is declared as an ordinary
// (thiscall) member, but its mangled name is one of the `QAA` "Win"
// window-procedure family that recovery_symbols.py documents as omitting
// the receiver: the call site here pushes all ten arguments on the stack
// and never loads ecx. A thiscall invocation would insert a `mov ecx, ...`
// this call does not have, so the call is spelled through a fresh
// extern "C" alias instead of the emitted member (rule 4: shadow
// declarations above the definition are kept).
extern "C" int __cdecl fn_gen_terrain_poly(Buffer *, MapWin *, int, int, int,
                                           int, int, int, int, int);

// The subject's own mangled name is the SAME `QAA` Win-family shape and the
// disassembly confirms it: every parameter is read at [ebp+8]..[ebp+0x18]
// with no receiver in ecx or on the stack. The class above declares
// `terrain_draw` as a non-static __cdecl MEMBER, and a __cdecl member still
// passes `this` - pushed onto the stack as a hidden first argument, not
// through ecx - which shifts every real parameter's frame offset by +4
// (measured: [ebp+0xc] compiled as [ebp+0x10], etc: SHAPE_EXACT but not
// BYTE_EXACT). Defined here as a free function with the identical
// parameter list instead, which is what the bytes actually show; the
// class's own (unused) declaration is left standing since nothing may be
// removed from the context.
void __cdecl terrain_draw(Sprite* a1, GraphicWin* a2, int a3, int a4, int a5) {
    Buffer *buf;
    if (a2) {
        buf = reinterpret_cast<Buffer *>(reinterpret_cast<char *>(a2) + 0x444);
    } else {
        buf = 0;
    }
    fn_gen_terrain_poly(buf, 0, 0, *g_00939284, -1, 0x621, *g_008cc1e4,
                        *g_008cc1e8, a4, a5);
}
