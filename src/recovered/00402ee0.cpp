// ORIGINAL: 0x00402EE0 sub_402ee0 0x00402EE0-0x00402F08 BYTE_EXACT
// size      40 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00402EF7
// 0x00402EE0  sub_402ee0  ->  _sub_402ee0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

#pragma warning(disable:4716)
class Dummy402ee0 { public: void mfn(); };
typedef void (Dummy402ee0::*Pfn402ee0)();

extern "C" int __stdcall sub_402ee0(int a1, int a2, int a3, int a4) {
    int i;
    if (--a3 >= 0) {
        i = a3 + 1;
        do {
            Pfn402ee0 fn = *reinterpret_cast<Pfn402ee0 *>(&a4);
            Dummy402ee0 *p = reinterpret_cast<Dummy402ee0 *>(a1);
            (p->*fn)();
            a1 = a1 + a2;
        } while (--i);
    }
}
