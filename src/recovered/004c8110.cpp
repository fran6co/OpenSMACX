// ORIGINAL: 0x004C8110
// 0x004C8110  sub_4c8110  ->  _sub_4c8110
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void *__cdecl _memset(void*, int, unsigned int);

class LegacyBlock { public:
    LegacyBlock();
};

LegacyBlock::LegacyBlock() {
    char *self = reinterpret_cast<char *>(this);
    _memset(self + 4, 0, 4);
    *reinterpret_cast<int *>(self) = 0;
    *reinterpret_cast<int *>(self + 8) = 0;
    *reinterpret_cast<int *>(self + 0xc) = 0;
}
