// ORIGINAL: 0x00625380
// 0x00625380  sub_625380  ->  _sub_625380
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class ButtonInit { public:
    int sub_625380(int, int, int, int, int, int, int);
};

int ButtonInit::sub_625380(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
    if (a1 == 0) {
        return 3;
    }
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0xab8) = a1;
    char *p = reinterpret_cast<char *>(a1);
    int v1c = *reinterpret_cast<int *>(p + 0x1c);
    int v18 = *reinterpret_cast<int *>(p + 0x18);
    BaseButton *bb = reinterpret_cast<BaseButton *>(this);
    return bb->init(reinterpret_cast<char *>(a2), a3, a4, a5, v18, v1c, reinterpret_cast<Win *>(a6), a7);
}
