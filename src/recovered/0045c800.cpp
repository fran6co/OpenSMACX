// ORIGINAL: 0x0045C800 ?on_button_clicked@MainInterface@@QAEXH@Z 0x0045C800-0x0045C830 BYTE_EXACT
// BLOCKED 2026-09-04, not on the body: the body homes byte-exact (15 of 15
//         instructions, /c /O2 /Gy /GR- /Oy- /GX) once its four fixed-address
//         bindings are renamed to the names maininterface.cpp already held -
//         WorldClimateSkipTerrainClear (map.h, 0x0068F21C), ActiveIfaceId
//         (0x007D392C), IfaceIdToSlot (0x007AE778), IfaceRegistry
//         (0x007AE7F0). It was reverted whole for one reason: the dispatch
//         through the shim class costs two more `VCall` mentions and
//         compiler_work's VCall-shim ceiling is at its floor (32 of 32).
//         slot013 is declared nullary in that class and unused; giving it the
//         int in place is the one-line edit, but the census still counts the
//         call site. The census's own remedy is 'declare the method virtual',
//         and the class that owns IfaceRegistry's entries is not modelled -
//         its pass retires the whole family, this body with it. Re-home then.
// size      48 bytes
// prototype void (__thiscall ?on_button_clicked@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C829
// 0x0045C800  ?on_button_clicked@MainInterface@@QAEXH@Z  ->  ?on_button_clicked@MainInterface@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Second VCall shim: slot013 here takes an int, unlike the emitter's
// nullary default for every slot.
class VCallI { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013(int);
};

void MainInterface::on_button_clicked(int a1) {
    if (*g_0068f21c != 1) {
        int index = *g_007d392c;
        int subindex = reinterpret_cast<int *>(g_007ae778)[index];
        VCallI *target = reinterpret_cast<VCallI **>(g_007ae7f0)[subindex];
        if (target != 0) {
            target->slot013(a1);
        }
    }
}
