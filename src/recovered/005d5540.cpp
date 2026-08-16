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
// ORIGINAL: 0x005D5540
// name      ?resize@GraphicWin@@QAEXHHH@Z
// size      80 bytes
// spans     0x005D5540-0x005D5590
// prototype void (__thiscall ?resize@GraphicWin@@QAEXHHH@Z)(GraphicWin* this, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7670 0x005ED880 0x005EEF60
// 0x005D5540  ?resize@GraphicWin@@QAEXHHH@Z  ->  ?resize@GraphicWin@@QAEXHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void GraphicWin::resize(int a1, int a2, int a3) {
    if (a3 == 0) {
        reinterpret_cast<Win *>(this)->nonclient_to_client(&a1, &a2);
    }
    if (buffer_.init(a1, a2, 0, 0) == 0) {
        reinterpret_cast<Win *>(this)->resize(a1, a2, 1);
    }
}
