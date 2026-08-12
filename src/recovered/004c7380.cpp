// ORIGINAL: 0x004C7380 BYTE_EXACT
// name      sub_4c7380
// size      51 bytes
// spans     0x004C7380-0x004C73B3
// prototype 
// callers   0   call targets   1
// 0x004C7380  sub_4c7380  ->  _sub_4c7380
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class ArrayOwner { public:
    unsigned int shrink(int *a1);
};

unsigned int ArrayOwner::shrink(int *a1) {
    void **self = reinterpret_cast<void **>(this);
    unsigned int index = (unsigned int)a1[1];
    unsigned int count = (unsigned int)a1[0];
    void *victim = self[index];
    ++count;
    operator delete(victim);
    if (index < count) {
        for (unsigned int i = index; i < count; ++i) {
            self[i] = 0;
        }
    }
    return count;
}
