// ORIGINAL: 0x00407C60 FILE
// name      ?hide@AlphaSave@@QAEXXZ
// size      100 bytes
// spans     0x00407C60-0x00407CC4
// prototype void (__thiscall ?hide@AlphaSave@@QAEXXZ)(AlphaSave* this)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D970 0x0045E540 0x005EDCD0 0x005F7E90
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00407C60
// measured tier  NO_COMPILE
// refusal        u00407c60.cpp(24) : error C2653: 'AlphaSave' : is not a class or namespace name u00407c60.cpp(25) : error C2061: syntax error : identifier 'Win' u00407c60.cpp(2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00407c60/unit.cpp
// and score it with tools/agent_brief.py.
// 0x00407C60  ?hide@AlphaSave@@QAEXXZ  (100 bytes)
//
// AlphaSave is an opaque shell over a Win/GraphicWin-shaped object far
// larger than either's own recovered layout (fields at 0x3280/0x3318/0x3dd8,
// well past GraphicWin's ~0xa14 bytes) - reached by raw offset, per the
// class's own emptiness.
//
// `this` is reused directly as `Win *` for BOTH `is_visible()` calls and the
// final `hide()` call: the disassembly never adjusts `ecx`/`esi` between
// them, so each is `reinterpret_cast<Win *>(this)->method()`, not a real
// base-class call.
//
// The second `is_visible()` target is computed as a self-relative pointer:
// `x = *g_009156b0` (the stored value), `y = *(int*)(x+4)`, then
// `y + (address of g_009156b0 itself)` recovers an absolute pointer from a
// base-relative offset. Because `y` is a runtime value, the trailing
// `+ 0x9156b0` can never be constant-folded away regardless of how the
// global is spelled - the given `static int *const g_009156b0` (used via
// `reinterpret_cast<int>(g_009156b0)` for its address, not a redeclared
// `extern`) already reproduces the `add ecx, imm32`. This is a case the
// general "address itself does work" lever does NOT force an override for:
// the fold it warns about needs the WHOLE computation to be compile-time
// constant, and here it never is.
void AlphaSave::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        int x = *g_009156b0;
        int y = *reinterpret_cast<int *>(x + 4);
        Win *other = reinterpret_cast<Win *>(y + reinterpret_cast<int>(g_009156b0));
        if (other->is_visible()) {
            char *self = reinterpret_cast<char *>(this);
            void *p1 = *reinterpret_cast<void **>(self + 0x3280);
            void *p2 = *reinterpret_cast<void **>(reinterpret_cast<char *>(p1) + 4);
            unsigned char flags = *reinterpret_cast<unsigned char *>(
                reinterpret_cast<char *>(p2) + reinterpret_cast<int>(self) + 0x3318);
            if (flags & 0xc) {
                scroll_hide(reinterpret_cast<GraphicWin *>(self),
                            reinterpret_cast<Scroll *>(self + 0x3dd8));
            }
            slide_hide(reinterpret_cast<GraphicWin *>(self), 0);
        }
        *g_006a7524 = 0;
        reinterpret_cast<Win *>(this)->hide();
    }
}
