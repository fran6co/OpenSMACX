// ORIGINAL: 0x005FCD40
// 0x005FCD40  sub_5fcd40  ->  _sub_5fcd40
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct tagMSG { int _[7]; };

typedef int (__stdcall *fn_peek)(tagMSG *, void *, unsigned int, unsigned int, unsigned int);
typedef void (__stdcall *fn_translate)(tagMSG *);
typedef void (__stdcall *fn_dispatch)(tagMSG *);

extern "C" void __cdecl sub_5fcd40() {
    tagMSG msg;
    fn_translate translate = reinterpret_cast<fn_translate>(*g_0066935c);
    fn_peek peek = reinterpret_cast<fn_peek>(*g_00669358);
    fn_dispatch dispatch = reinterpret_cast<fn_dispatch>(*g_00669340);
    do {
        do_video();
        check_net();
    } while (peek(&msg, 0, 0x100, 0x108, 1) && (translate(&msg), dispatch(&msg), true));
    do_net();
}
