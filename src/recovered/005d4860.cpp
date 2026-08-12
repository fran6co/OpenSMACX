// ORIGINAL: 0x005D4860 BYTE_EXACT
// name      sub_5d4860
// size      39 bytes
// spans     0x005D4860-0x005D4887
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// 0x005D4860  sub_5d4860  ->  _sub_5d4860
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl _free(void*);

class LegacyValue { public:
    void release_value(int*);
};

void LegacyValue::release_value(int *a1) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x18) == 0) {
        if (a1[1] != 0) {
            _free(reinterpret_cast<void *>(a1[1]));
            a1[1] = 0;
        }
    }
}
