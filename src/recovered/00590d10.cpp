// ORIGINAL: 0x00590D10 BYTE_EXACT
// 0x00590D10  sub_590d10  ->  _sub_590d10
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef void (__cdecl *Callback)(int, int, int);
struct CallbackEntry {
    int unused;
    Callback callback;
};
class Subject { public:
    void invoke(int, int);
};

void Subject::invoke(int a1, int a2) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    char *self = reinterpret_cast<char *>(this);
    int first = static_cast<unsigned char>(self[2]);
    CallbackEntry *entry = reinterpret_cast<CallbackEntry *>(g_00945a60) + first;
    if (entry->callback != 0) {
        entry->callback(a1, static_cast<unsigned char>(self[3]), a2);
    }
}
