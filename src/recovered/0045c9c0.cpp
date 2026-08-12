// ORIGINAL: 0x0045C9C0 BYTE_EXACT
// name      ?on_mouse_leave@MainInterface@@QAEXHH@Z
// size      52 bytes
// spans     0x0045C9C0-0x0045C9F4
// prototype void (__thiscall ?on_mouse_leave@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x0045C9C0  ?on_mouse_leave@MainInterface@@QAEXHH@Z  ->  ?on_mouse_leave@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Second VCall shim: slot009 here takes two ints, unlike the emitter's
// nullary default for every slot.
class VCallII { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009(int, int);
};

void MainInterface::on_mouse_leave(int a1, int a2) {
    if (*g_0068f21c != 1) {
        int index = *g_007d392c;
        int subindex = reinterpret_cast<int *>(g_007ae778)[index];
        VCallII *target = reinterpret_cast<VCallII **>(g_007ae7f0)[subindex];
        if (target != 0) {
            target->slot009(a1, a2);
        }
    }
}
