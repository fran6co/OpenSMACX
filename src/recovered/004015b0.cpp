// 0x004015B0  ?seek_pos@StringStruct@@QAEHH@Z  ->  ?seek_pos@StringStruct@@QAEHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__cdecl *SeekAbsFn)(int);

int StringStruct::seek_pos(int a1) {
    int *s = reinterpret_cast<int *>(this);
    if (a1 > s[4] - 1) {
        return 0;
    }
    s[3] = s[2];
    if (a1 < 0) {
        if (reinterpret_cast<SeekAbsFn>(abs)(a1) > s[4]) {
            return 0;
        }
        int i = reinterpret_cast<SeekAbsFn>(abs)(a1);
        while (i > 0) {
            i--;
            s[3] = reinterpret_cast<int *>(s[3])[4];
        }
        a1 += s[4];
    } else {
        int j = a1;
        while (j > 0) {
            j--;
            s[3] = reinterpret_cast<int *>(s[3])[3];
        }
    }
    s[5] = a1;
    return 1;
}
