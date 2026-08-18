// ORIGINAL: 0x004C7CC0 sub_4c7cc0 0x004C7CC0-0x004C7CEA BYTE_EXACT
// size      42 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C7CC0
// 0x004C7CC0  sub_4c7cc0  ->  _sub_4c7cc0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class LegacyNode { public:
    int set_flag(int);
};

int LegacyNode::set_flag(int a1) {
    char *self = reinterpret_cast<char *>(this);
    uint8_t &value = *reinterpret_cast<uint8_t *>(self + 0x54);
    value ^= (value ^ static_cast<uint8_t>(a1)) & 1;
    LegacyNode *child =
        *reinterpret_cast<LegacyNode **>(self + 0x3c);
    if (child != 0) {
        child->set_flag(a1);
    }
    return 0;
}
