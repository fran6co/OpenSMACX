// ORIGINAL: 0x004016C0 BYTE_EXACT
// 0x004016C0  ?close@ServiceStruct@@QAEXXZ  ->  ?close@ServiceStruct@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class VCallX { public:
    virtual void slot000(int);
    virtual void slot001(void *);
};

void ServiceStruct::close() {
    char *base = reinterpret_cast<char *>(this) - 0x1c;
    *reinterpret_cast<int **>(base) = g_006693d4;
    int *vbt = *reinterpret_cast<int **>(reinterpret_cast<char *>(this) - 0x18);
    *reinterpret_cast<int **>(reinterpret_cast<char *>(this) - 0x18 + vbt[1]) = g_006693d0;
    if (*reinterpret_cast<char **>(base + 8) != 0) {
        int i = 0;
        if (*reinterpret_cast<int *>(base + 0x10) > 0) {
            do {
                char *node = *reinterpret_cast<char **>(base + 8);
                *reinterpret_cast<char **>(base + 0xc) = *reinterpret_cast<char **>(node + 0xc);
                char *data = *reinterpret_cast<char **>(node + 8);
                reinterpret_cast<VCallX *>(base)->slot001(data);
                if (data != 0) {
                    char *adj = data + (*reinterpret_cast<int **>(data))[1];
                    reinterpret_cast<VCallX *>(adj)->slot000(1);
                }
                *reinterpret_cast<char **>(*reinterpret_cast<char **>(base + 8) + 8) = 0;
                char *n2 = *reinterpret_cast<char **>(base + 8);
                if (n2 != 0) {
                    reinterpret_cast<VCallX *>(n2 + (*reinterpret_cast<int **>(n2))[1])->slot000(1);
                }
                *reinterpret_cast<char **>(base + 8) = *reinterpret_cast<char **>(base + 0xc);
                i++;
            } while (i < *reinterpret_cast<int *>(base + 0x10));
        }
        *reinterpret_cast<char **>(base + 8) = 0;
        *reinterpret_cast<char **>(base + 0x14) = 0;
        *reinterpret_cast<char **>(base + 0x10) = 0;
    }
    *reinterpret_cast<char **>(base + 0x14) = 0;
}
