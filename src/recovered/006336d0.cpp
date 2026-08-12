// ORIGINAL: 0x006336D0 BYTE_EXACT
// name      sub_6336d0
// size      65 bytes
// spans     0x006336D0-0x00633711
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00633720 0x00644EF2
// 0x006336D0  sub_6336d0  ->  _sub_6336d0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class Sub6336d0Owner { public:
    void *sub_6336d0(unsigned char a1);
};

void *Sub6336d0Owner::sub_6336d0(unsigned char a1) {
    j_VoiceRx__VoiceRx_dt();
    int saved = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x5c);
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x58) = 0x6693ac;
    *g_009b3374 = saved;
    if ((a1 & 1) && this != 0) {
        if (saved == 0) {
            free(this);
        }
        *g_009b3374 = 0;
    }
    return this;
}
