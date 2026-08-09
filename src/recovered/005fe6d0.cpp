// ORIGINAL: 0x005FE6D0
// 0x005FE6D0  ?reseed@Palette@@QAEXXZ  ->  ?reseed@Palette@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Palette::reseed() {
    char *self = reinterpret_cast<char *>(this);
    uint32_t *seed = reinterpret_cast<uint32_t *>(self + 0x400);
    *seed = 0;
    while (*seed == 0) {
        *seed = random(0, 0xffff);
    }
}
