// CLAIM DROPPED 2026-08-16, and the body is untouched. `Buffer` became
// polymorphic - `virtual ~Buffer()` and `virtual surface_lost()` in place
// of the explicit `LPVOID vtable_` member - which is what the image has
// and what lets a slot-1 call be spelled `surface_lost()`. The scaffold
// cannot lay out a class with virtuals, so it emits `Buffer` opaque here
// and this body stops compiling: `C2065: 'surface_' : undeclared`.
//
// The body is not wrong and was byte-exact the day before. What is gone
// is the ability to CHECK it from a scaffold, so the claim goes with it -
// state is measured, not remembered. It comes back the day this body is
// promoted into the file that owns its class, which is where it belongs.
// ORIGINAL: 0x005DDF60
// name      ?wrap_flying@Buffer@@QAEHPADHHH@Z
// size      70 bytes
// spans     0x005DDF60-0x005DDFA6
// prototype int (__thiscall ?wrap_flying@Buffer@@QAEHPADHHH@Z)(Buffer* this, int8*, int, int, int)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DDFB0
// 0x005DDF60  ?wrap_flying@Buffer@@QAEHPADHHH@Z  ->  ?wrap_flying@Buffer@@QAEHPADHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Buffer::wrap_flying(char *a1, int a2, int a3, int a4) {
    uint32_t saved = field_10_;
    field_18_ = 0;
    field_14_ = saved;
    *g_009b3a90 = a2;
    *g_009b3a9c = 0;
    *g_009b3a94 = a3;
    *g_009b3a98 = a4;
    field_51C_ = 0;
    *reinterpret_cast<uint8_t *>(g_009b33cc) = 0;
    return wrap_flying(a1);
}
