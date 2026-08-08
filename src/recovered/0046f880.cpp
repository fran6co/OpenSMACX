// 0x0046F880  ?on_sys_close@MapWin@@QAEXXZ  ->  ?on_sys_close@MapWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl _free(void *);

void MapWin::on_sys_close() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self - 0x3cf4)) {
        char *group = self - 0x21a6c;
        *reinterpret_cast<int *>(self - 0x3cf8) = 0;
        void *p = *reinterpret_cast<void **>(group + 4);
        if (p) {
            _free(p);
            *reinterpret_cast<void **>(group + 4) = 0;
        }
        int *vtable = *reinterpret_cast<int **>(group);
        reinterpret_cast<GraphicWin *>(group + vtable[1])->close();
    }
}
