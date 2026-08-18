// ORIGINAL: 0x0042BEB0 ?UNK7@Datalink@@QAEXPADPAD@Z 0x0042BEB0-0x0042BEFA BYTE_EXACT
// size      74 bytes
// prototype void (__thiscall ?UNK7@Datalink@@QAEXPADPAD@Z)(Datalink* this, int8*, int8*)
// callers   0   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00608980 0x00629490 0x00629780
// indirect  0x0042BEF1
// 0x0042BEB0  ?UNK7@Datalink@@QAEXPADPAD@Z  ->  ?UNK7@Datalink@@QAEXPAD0@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Shadow shim: same vtable layout as the emitted VCall (two slots), but
// slot001 takes the int argument the call site pushes.
class VCallInt { public:
    virtual void slot000();
    virtual void slot001(int);
};

void Datalink::UNK7(char * a1, char * a2) {
    if (*g_007d3930 != 0) {
        char *self = reinterpret_cast<char *>(this);
        StringBox *box = reinterpret_cast<StringBox *>(self + 0x130b4);
        StringList *list = reinterpret_cast<StringList *>(self + 0x15c24);
        box->clear();
        list->load(a1, a2, 1, 0);
        box->add_fixup();
        reinterpret_cast<VCallInt *>(box)->slot001(0);
    }
}
