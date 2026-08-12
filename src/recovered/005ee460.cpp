// ORIGINAL: 0x005EE460 BYTE_EXACT
// 0x005EE460  sub_5ee460  ->  _sub_5ee460
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef void (__cdecl *Callback)();

// Same layout as the generated VCall shim above, except slot023 returns the
// value the original tests in eax. Declaration order (not the return type)
// is what fixes a slot's vtable index, so this is free to diverge from
// VCall on that one signature.
class VCall2 { public:
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
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual int slot023();
};

class Obj { public:
    void sub_5ee460();
};

void Obj::sub_5ee460() {
    char *self = reinterpret_cast<char *>(this);
    *g_009b7ab8 = reinterpret_cast<int>(this);
    Callback callback = *reinterpret_cast<Callback *>(self + 0x404);
    if (callback != 0) {
        callback();
    }
    reinterpret_cast<VCall2 *>(this)->slot014();
    VCall2 *second = *reinterpret_cast<VCall2 **>(self + 0x18);
    if (second != 0) {
        if (second->slot023() == 0) {
            second->slot007();
        }
    }
}
