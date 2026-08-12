// ORIGINAL: 0x0060FA80 BYTE_EXACT
// name      ?on_left_double_click@CheckBox@@QAEXHH@Z
// size      45 bytes
// spans     0x0060FA80-0x0060FAAD
// prototype void (__thiscall ?on_left_double_click@CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   1   call targets   0
// 0x0060FA80  ?on_left_double_click@CheckBox@@QAEXHH@Z  ->  ?on_left_double_click@CheckBox@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void CheckBox::on_left_double_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int disp1 = (*reinterpret_cast<int **>(self - 0x1c))[1];
    *g_009b7ab8 = *reinterpret_cast<int *>(self + disp1 + 0xa8);

    int callback = *reinterpret_cast<int *>(self - 0x10);
    if (callback != 0) {
        int disp2 = (*reinterpret_cast<int **>(self - 0x1c))[2];
        int arg = *reinterpret_cast<int *>(self + disp2 + 0xd0);
        typedef int(__cdecl * Callback)(int);
        reinterpret_cast<Callback>(callback)(arg);
    }
}
