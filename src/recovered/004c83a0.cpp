// ORIGINAL: 0x004C83A0 BYTE_EXACT
// 0x004C83A0  sub_4c83a0  ->  _sub_4c83a0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void operator delete(void *);
void *operator new(unsigned int);
#pragma function(strlen, strcpy)

class Subject { public:
    void set_name(char *name);
};

void Subject::set_name(char *name) {
    char *self = reinterpret_cast<char *>(this);
    void *old = *reinterpret_cast<void **>(self + 0x24);
    if (old != 0) {
        operator delete(old);
    }
    if (name != 0) {
        unsigned int n = strlen(name) + 1;
        void *buf = operator new(n);
        *reinterpret_cast<void **>(self + 0x24) = buf;
        strcpy(static_cast<char *>(buf), name);
    } else {
        *reinterpret_cast<void **>(self + 0x24) = 0;
    }
}
