// ORIGINAL: 0x00634FE0 BYTE_EXACT
// 0x00634FE0  ?shutdown@TexHeap@@QAEXXZ  ->  ?shutdown@TexHeap@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void TexHeap::shutdown() {
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int *>(self) != 0) {
        free(reinterpret_cast<void *>(*reinterpret_cast<int *>(self)));
        *reinterpret_cast<int *>(self) = 0;
    }

    if (*reinterpret_cast<int *>(self + 8) != 0) {
        for (int i = 0; i < *reinterpret_cast<int *>(self + 0xc) + 1; i++) {
            int value = *reinterpret_cast<int *>(
                *reinterpret_cast<int *>(self + 8) + i * 4);
            if (value != 0) {
                free(reinterpret_cast<void *>(value));
                *reinterpret_cast<int *>(
                    *reinterpret_cast<int *>(self + 8) + i * 4) = 0;
            }
        }
        if (*reinterpret_cast<int *>(self + 8) != 0) {
            free(reinterpret_cast<void *>(*reinterpret_cast<int *>(self + 8)));
        }
        *reinterpret_cast<int *>(self + 8) = 0;
    }

    *reinterpret_cast<int *>(self + 0xc) = 0;
    *reinterpret_cast<int *>(self + 4) = 0;
}
