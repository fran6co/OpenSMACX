// ORIGINAL: 0x0062D1A0 sub_62d1a0 0x0062D1A0-0x0062D1D2 BYTE_EXACT
// TRIED: homed into general.cpp 2026-09-03 - does not link: the body reads and
// writes a counter at 0x009B3AB0 (extern C int dat_9b3ab0) that nothing in the
// tree defines. Defining it as a real global gives it a linker-chosen address
// rather than 0x009B3AB0, which is the layout half of the recovery and is not
// mine to invent; it stays here until that global is modelled.
// size      50 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0062D1C4
// 0x0062D1A0  sub_62d1a0  ->  _sub_62d1a0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__stdcall *Vfn26)(int, int);
extern "C" int dat_9b3ab0;
void __cdecl sub_62d1a0() {
    int v = dat_9b3ab0;
    int *obj = *(int **)g_009bc498;
    v--;
    dat_9b3ab0 = v;
    if (obj) {
        int handler = *g_009bc48c;
        if (handler) {
            ((Vfn26)(*(int **)obj)[26])((int)obj, handler);
            *g_009bc48c = 0;
        }
    }
}
