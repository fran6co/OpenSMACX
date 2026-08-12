// ORIGINAL: 0x00617670 BYTE_EXACT
// name      ?UNK2@Caviar@@QAEHPAUBuffer@@HHHH@Z
// size      74 bytes
// spans     0x00617670-0x006176BA
// prototype int (__thiscall ?UNK2@Caviar@@QAEHPAUBuffer@@HHHH@Z)(Caviar* this, Buffer*, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00617230
// 0x00617670  ?UNK2@Caviar@@QAEHPAUBuffer@@HHHH@Z  ->  ?UNK2@Caviar@@QAEHPAUBuffer@@HHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Caviar::UNK2(Buffer *a1, int a2, int a3, int a4, int a5) {
    if (a4 == 0) {
        return 0x10;
    }
    *g_009bb474 = a4;
    int *flags = reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 4);
    *flags |= 0x10;
    int result = render(a1, a2, a3, a5);
    *flags &= 0xffffffef;
    return result;
}
