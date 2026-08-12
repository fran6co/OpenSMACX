// ORIGINAL: 0x00615EC0 BYTE_EXACT
// 0x00615EC0  ?on_left_up@EditBox@@QAEXHH@Z  ->  ?on_left_up@EditBox@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class EditBoxTP { public:
    int text_position(int);
};

void EditBox::on_left_up(int a1, int a2) {
    void *active = *reinterpret_cast<void **>(g_009b7abc);
    if (active == this) {
        *g_009b7abc = 0;
        reinterpret_cast<VCall *>(this)->slot004();
    }
    if (*reinterpret_cast<void **>(g_009b7ac0) == this) {
        *g_009b7ac0 = 0;
    }
    int pos = reinterpret_cast<EditBoxTP *>(this)->text_position(a1);
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xb44) = pos;
    reinterpret_cast<VCall *>(this)->slot062();
}
