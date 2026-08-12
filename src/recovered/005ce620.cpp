// ORIGINAL: 0x005CE620 BYTE_EXACT
// name      sub_5ce620
// size      31 bytes
// spans     0x005CE620-0x005CE63F
// prototype 
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005CE62F
// 0x005CE620  sub_5ce620  ->  _sub_5ce620
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ecx is read as an object pointer with no stack-passed arguments, so
// this is a __thiscall member of some still-unidentified class - not
// really a free `__cdecl` function as the generic fallback name implies.
//
// The dispatch pushes the object pointer explicitly rather than
// putting it in ECX, so - like 0x0062EDB0 - it is a manual __stdcall
// table call (slot 2, offset 8), not a thiscall v-call.
typedef void (__stdcall *Slot2Fn)(void *);

class Unk5ce620 { public:
    int method();
};

int Unk5ce620::method() {
    char *holder = *reinterpret_cast<char **>(this);
    void *obj = *reinterpret_cast<void **>(holder + 0x5c);
    if (obj) {
        Slot2Fn fn = reinterpret_cast<Slot2Fn *>(*reinterpret_cast<void **>(obj))[2];
        fn(obj);
        char *holder2 = *reinterpret_cast<char **>(this);
        *reinterpret_cast<int *>(holder2 + 0x5c) = 0;
    }
    return 0;
}
