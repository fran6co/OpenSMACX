// ORIGINAL: 0x005EE4A0
// 0x005EE4A0  sub_5ee4a0  ->  _sub_5ee4a0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The emitted VCall above declares every slot nullary. Slots 15 and 30 are
// actually called with one int argument (`push edx` then `call [eax+0x3c]`
// / `call [eax+0x78]`), so a second shim carries the real arities. Only
// the two used slots need the right signature - order fixes the index.
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
    virtual void slot015(int);  // <-- used
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030(int);  // <-- used
};

// `[ecx+0x98]` and `[ecx+0x9c]` are read with no stack access to reach
// them, and the two stack slots (`[esp+4]`, `[esp+8]`) plus `ret 8` are
// exactly a __thiscall taking two ints - so the subject is a member, not
// the free __stdcall function the default scaffold proposes.
class Subject { public:
    void invoke(int, int);
};

void Subject::invoke(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if ((*reinterpret_cast<int *>(self + 0x98) & 0x200000) == 0 &&
        (*reinterpret_cast<unsigned char *>(self + 0x9c) & 8) == 0) {
        VCall2 *target = reinterpret_cast<VCall2 *>(this);
        if (a2 == 0) {
            target->slot015(a1);
        } else {
            target->slot030(a1);
        }
    }
}
