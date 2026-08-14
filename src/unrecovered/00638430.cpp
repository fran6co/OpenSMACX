// ORIGINAL: 0x00638430
// name      sub_638430
// size      413 bytes
// spans     0x00638430-0x006385CD
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00638447 0x00638458 0x00638469 0x0063847A 0x0063848B 0x006384AB 0x006384C0 0x006384E6 0x006384FB 0x00638510 0x00638528 0x0063854D 0x00638562 0x00638589 0x0063859E 0x006385B6 0x006385C0
// RULED-OUT: teardown - frees five plain pointer fields (0xc/0x10/0x14/
//            0x18/0x1c), three arrays of small structs at 0x28/0x2c/0x30/
//            0x34 sized by a shared count field at 0x24, then the object
//            itself through the free callback at g_009c0b84. Diverges at
//            #6 - the original re-reads each array-base field inside the
//            loop rather than caching it once; landed at 0.64 similarity.

typedef void (__cdecl *FreeFn)(void *);

extern "C" void __cdecl sub_638430(void *a1) {
    if (a1 == 0) {
        return;
    }

    FreeFn freeFn = reinterpret_cast<FreeFn>(*g_009c0b84);
    char *base = reinterpret_cast<char *>(a1);

    void *p;

    p = *reinterpret_cast<void **>(base + 0xc);
    if (p != 0) freeFn(p);

    p = *reinterpret_cast<void **>(base + 0x10);
    if (p != 0) freeFn(p);

    p = *reinterpret_cast<void **>(base + 0x14);
    if (p != 0) freeFn(p);

    p = *reinterpret_cast<void **>(base + 0x18);
    if (p != 0) freeFn(p);

    p = *reinterpret_cast<void **>(base + 0x1c);
    if (p != 0) freeFn(p);

    char *arr28 = *reinterpret_cast<char **>(base + 0x28);
    if (arr28 != 0) {
        int count = *reinterpret_cast<int *>(base + 0x24);
        for (int i = 0; i < count; i++) {
            void *elem = *reinterpret_cast<void **>(arr28 + i * 4);
            if (elem != 0) {
                freeFn(elem);
            }
        }
        freeFn(arr28);
    }

    char *arr30 = *reinterpret_cast<char **>(base + 0x30);
    if (arr30 != 0) {
        int count = *reinterpret_cast<int *>(base + 0x24);
        for (int i = 0; i < count; i++) {
            char *elem = arr30 + i * 0xc;
            void *p0 = *reinterpret_cast<void **>(elem);
            if (p0 != 0) freeFn(p0);
            void *p1 = *reinterpret_cast<void **>(elem + 4);
            if (p1 != 0) freeFn(p1);
            void *p2 = *reinterpret_cast<void **>(elem + 8);
            if (p2 != 0) freeFn(p2);
        }
        freeFn(arr30);
    }

    char *arr2c = *reinterpret_cast<char **>(base + 0x2c);
    if (arr2c != 0) {
        int count = *reinterpret_cast<int *>(base + 0x24);
        for (int i = 0; i < count; i++) {
            void *p1 = *reinterpret_cast<void **>(arr2c + i * 8 + 4);
            if (p1 != 0) freeFn(p1);
        }
        freeFn(arr2c);
    }

    char *arr34 = *reinterpret_cast<char **>(base + 0x34);
    if (arr34 != 0) {
        int count = *reinterpret_cast<int *>(base + 0x24);
        for (int i = 0; i < count; i++) {
            char *elem = arr34 + i * 0xc;
            void *p1 = *reinterpret_cast<void **>(elem + 4);
            if (p1 != 0) freeFn(p1);
            void *p2 = *reinterpret_cast<void **>(elem + 8);
            if (p2 != 0) freeFn(p2);
        }
        freeFn(arr34);
    }

    freeFn(a1);
}
