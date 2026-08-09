// ORIGINAL: 0x00590D70
// 0x00590D70  sub_590d70  ->  _sub_590d70
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// SIGNATURE CHANGE: the original reads [ecx+0x200] with no stack access to
// find the receiver, so ecx is `this`, not a stack argument - this is a
// __thiscall member, not the __stdcall free function the scaffold guessed.
// `ret 0xc` is the 3 explicit stack args (short, signed char, signed char)
// popped by thiscall, not a 3-int stdcall arg list.
class RecordLog590d70 { public:
    void append(short a2, signed char a3, signed char a4);
};

void RecordLog590d70::append(short a2, signed char a3, signed char a4) {
    char *self = reinterpret_cast<char *>(this);
    int *count = reinterpret_cast<int *>(self + 0x200);
    if (*count < 0x80) {
        *reinterpret_cast<short *>(self + *count * 4) = a2;
        *reinterpret_cast<signed char *>(self + *count * 4 + 2) = a3;
        *reinterpret_cast<signed char *>(self + *count * 4 + 3) = a4;
        *count = *count + 1;
    }
}
