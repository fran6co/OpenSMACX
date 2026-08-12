// ORIGINAL: 0x00590CF0 BYTE_EXACT
// name      sub_590cf0
// size      31 bytes
// spans     0x00590CF0-0x00590D0F
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00590D06
// 0x00590CF0  sub_590cf0  ->  _sub_590cf0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__cdecl *Callback)(int, int, int);

class Obj590cf0 { public:
    int method(int, int, int);
};

int Obj590cf0::method(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    int fn = *reinterpret_cast<int *>(self + 4);
    if (fn != 0) {
        fn = (reinterpret_cast<Callback>(fn))(a1, a2, a3);
    }
    return fn;
}
