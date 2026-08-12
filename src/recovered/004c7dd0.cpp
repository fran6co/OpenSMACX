// ORIGINAL: 0x004C7DD0 BYTE_EXACT
// name      ?load@Midi@@QAEHXZ
// size      75 bytes
// spans     0x004C7DD0-0x004C7E1B
// prototype int (__thiscall ?load@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C6280
// indirect  0x004C7DFB
// 0x004C7DD0  ?load@Midi@@QAEHXZ  ->  ?load@Midi@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Midi::load() {
    char *self = reinterpret_cast<char *>(this);
    const char *filename = *reinterpret_cast<const char **>(self + 0x4c);
    if (filename != 0) {
        if (field_3c_ == 0) {
            if (*g_0090db7c == 0) {
                return 1;
            }
            typedef int (__cdecl *LoadDeviceFn)(uint32_t *, const char *, int);
            int result = reinterpret_cast<LoadDeviceFn>(*g_0090db24)(&field_3c_, filename, 2);
            if (result != 0) {
                return result;
            }
        }
        return reinterpret_cast<Sound *>(this)->load(filename);
    }
    return 8;
}
