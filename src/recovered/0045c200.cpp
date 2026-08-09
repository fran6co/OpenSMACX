// ORIGINAL: 0x0045C200
// 0x0045C200  ?timer_callback_daemon@MainInterface@@QAAXHH@Z  ->  ?timer_callback_daemon@MainInterface@@QAAXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__stdcall *UnionRectProc)(RECT *, const RECT *, const RECT *);

// Both stack params are read directly ([ebp+8], [ebp+0xc]) with no `this`
// anywhere - defining the scaffold's MainInterface member out-of-line would
// add an implicit `this` ahead of them, shifting both to +0xc/+0x10
// (measured: SHAPE_EXACT with exactly those two `mov` constants off by 4).
// So this is a free function; the declared member is left undefined.
void __cdecl timer_callback_daemon(int a1, int a2) {
    if (a2 == 0) return;

    char *self = reinterpret_cast<char *>(a2);
    switch (a1) {
    case 0:
        reinterpret_cast<Time *>(self + 0x25040)->stop();
        break;
    case 1: {
        int *count = reinterpret_cast<int *>(self + 0x216f0);
        if (*count == -1) {
            reinterpret_cast<Time *>(self + 0x25068)->stop();
            return;
        }
        *count = *count + 1;

        RECT rc;
        (*reinterpret_cast<UnionRectProc *>(g_00669328))(
            &rc, reinterpret_cast<RECT *>(self + 0xde0),
            reinterpret_cast<RECT *>(self + 0xdf0));
        reinterpret_cast<GraphicWin *>(self)->soft_update(&rc);
        break;
    }
    }
}
