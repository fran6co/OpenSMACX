// 0x00401060  ?close@StringStruct@@QAEXXZ  ->  ?close@StringStruct@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class VCallX { public:
    virtual void slot000(int);
    virtual void slot001(void *);
};

void StringStruct::close() {
    char *base = reinterpret_cast<char *>(this) - 0x1c;
    *reinterpret_cast<int **>(base) = reinterpret_cast<int *>(0x006693A4);
    int *vbt = *reinterpret_cast<int **>(reinterpret_cast<char *>(this) - 0x18);
    *reinterpret_cast<int **>(reinterpret_cast<char *>(this) - 0x18 + vbt[1]) = reinterpret_cast<int *>(0x006693A0);
    if (*reinterpret_cast<char **>(base + 8) != 0) {
        int i = 0;
        if (*reinterpret_cast<int *>(base + 0x10) > 0) {
            do {
                char *node = *reinterpret_cast<char **>(base + 8);
                *reinterpret_cast<char **>(base + 0xc) = *reinterpret_cast<char **>(node + 0xc);
                void *data = *reinterpret_cast<char **>(node + 8);
                VCallX *self = reinterpret_cast<VCallX *>(base);
                self->slot001(data);
                if (data != 0) {
                    char *adj = static_cast<char *>(data) + (*reinterpret_cast<int **>(data))[1];
                    reinterpret_cast<VCallX *>(adj)->slot000(1);
                }
                *reinterpret_cast<char **>(*reinterpret_cast<char **>(base + 8) + 8) = 0;
                char *n2 = *reinterpret_cast<char **>(base + 8);
                if (n2 != 0) {
                    reinterpret_cast<VCallX *>(n2 + (*reinterpret_cast<int **>(n2))[1])->slot000(1);
                }
                ++i;
                *reinterpret_cast<char **>(base + 8) = *reinterpret_cast<char **>(base + 0xc);
            } while (i < *reinterpret_cast<int *>(base + 0x10));
        }
        *reinterpret_cast<char **>(base + 8) = 0;
        *reinterpret_cast<char **>(base + 0x14) = 0;
        *reinterpret_cast<char **>(base + 0x10) = 0;
    }
    *reinterpret_cast<char **>(base + 0x14) = 0;
}
