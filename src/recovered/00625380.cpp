// ORIGINAL: 0x00625380 sub_625380 0x00625380-0x006253C6 BYTE_EXACT
// size      70 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00607210
// 0x00625380  sub_625380  ->  _sub_625380
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
//
// HANDOFF - this body does NOT home to texture.cpp. The nearest-marker
// shortlist proposed texture.cpp only because 0x00625380 sits immediately
// after sub_625080 (0x00625080). The COMDAT says otherwise:
//   * 0x0062539A stores [ecx + 0xAB8]. `sizeof(BaseButton) == 0xAB8`
//     (src/basebutton.h:114), so `this` is a class whose BaseButton
//     subobject ENDS at 0xAB8 and the store is that class's own first field
//     behind it. A Texture is 0x70 bytes.
//   * 0x006253BD calls 0x00607210, BaseButton::init, with `this` still in
//     ecx - the receiver is handed straight to the base's initialiser.
//   * `ret 0x1c` = seven stack dwords, and the null-argument early return
//     of 3 reads as a `?init@...@@QAEH...` on a BaseButton-derived window
//     class.
// The natural home is whichever TU owns that class - basewin.cpp /
// maininterface.cpp / basebutton.cpp - and the `ButtonInit` receiver below
// is the fiction standing in for it. The body itself is already
// transcribed; only the class needs naming.

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
