// 0x005D5590  sub_5d5590  ->  _sub_5d5590
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// This function reads [ecx] and [ecx+0x444] with no stack access, so
// the receiver is `this`, not a free function - proposed CATALOGUE
// change: extern "C" int __cdecl sub_5d5590() -> a __thiscall member
// (class unknown; Win's own fields end right at 0x444, so this is
// likely a direct Win subclass with an embedded Buffer there).
class Win3ArgVCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual int slot003(int, int, int);
};

class Sub5D5590Owner { public:
    int method();
};

int Sub5D5590Owner::method() {
    int result = reinterpret_cast<Win3ArgVCall *>(this)->slot003(*g_009b7b1c, *g_009b7b20, 0);
    if (result != 0) {
        return result;
    }
    reinterpret_cast<Win *>(this)->move(0, 0);
    reinterpret_cast<Buffer *>(reinterpret_cast<char *>(this) + 0x444)
        ->sync_to_palette(reinterpret_cast<Palette *>(*g_009b8180));
    return 0;
}
