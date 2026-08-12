// ORIGINAL: 0x0045C800 BYTE_EXACT
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
