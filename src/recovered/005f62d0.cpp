// ORIGINAL: 0x005F62D0 BYTE_EXACT
// name      sub_5f62d0
// size      71 bytes
// spans     0x005F62D0-0x005F6317
// prototype 
// callers   0   call targets   0
// 0x005F62D0  sub_5f62d0  ->  _sub_5f62d0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class VCallArg { public:
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
    virtual void slot015(int);
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
    virtual void slot030(int);
};

class Subject { public:
    void method(int a1, short a2, int a3, int a4, int a5);
};

void Subject::method(int a1, short a2, int a3, int a4, int a5) {
    int quotient = a2 / 120;
    char *self = reinterpret_cast<char *>(this);
    if (!(*reinterpret_cast<int *>(self + 0x98) & 0x200000) &&
        !(*reinterpret_cast<uint8_t *>(self + 0x9c) & 8)) {
        if (a5 == 0) {
            reinterpret_cast<VCallArg *>(this)->slot015(quotient);
        } else {
            reinterpret_cast<VCallArg *>(this)->slot030(quotient);
        }
    }
}
