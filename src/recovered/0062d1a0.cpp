// ORIGINAL: 0x0062D1A0 BYTE_EXACT
// name      sub_62d1a0
// size      50 bytes
// spans     0x0062D1A0-0x0062D1D2
// prototype 
// callers   0   call targets   0
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
