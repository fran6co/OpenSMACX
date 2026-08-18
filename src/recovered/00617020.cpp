// ORIGINAL: 0x00617020 ?close@Caviar@@QAEXXZ 0x00617020-0x0061707F BYTE_EXACT
// size      95 bytes
// prototype void (__thiscall ?close@Caviar@@QAEXXZ)(Caviar* this)
// callers   11   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// 0x00617020  ?close@Caviar@@QAEXXZ  ->  ?close@Caviar@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Caviar::close() {
    char *self = reinterpret_cast<char *>(this);
    char *record = self + 0x750;
    int count = 0xc8;
    do {
        int *slot0 = reinterpret_cast<int *>(record - 4);
        if (*slot0 != 0) {
            free(reinterpret_cast<void *>(*slot0));
            *slot0 = 0;
        }
        int *slot1 = reinterpret_cast<int *>(record);
        if (*slot1 != 0) {
            free(reinterpret_cast<void *>(*slot1));
            *slot1 = 0;
        }
        int *slot2 = reinterpret_cast<int *>(record + 4);
        if (*slot2 != 0) {
            free(reinterpret_cast<void *>(*slot2));
            *slot2 = 0;
        }
        record += 0xc;
    } while (--count);

    *reinterpret_cast<int *>(self + 4) = 0;
    scene_scale_ = 2.0f / 3.0f;
}
