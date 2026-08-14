// ORIGINAL: 0x0062E420
// name      ?on_player_added@Net@@QAEHPADI@Z
// size      278 bytes
// spans     0x0062E420-0x0062E536
// prototype int (__thiscall ?on_player_added@Net@@QAEHPADI@Z)(Net* this, int8*, unsigned int)
// callers   3   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D49D0 0x00644F3A 0x006465F0
// indirect  0x0062E4C4
// RULED-OUT: only one source-form tried. The mangled `a2` is `unsigned
//            int`, but the tail dereferences it as a struct pointer
//            (offsets +8/+0xc/+0x10/+0x14) - kept the catalogued type and
//            did the pointer arithmetic through casts rather than
//            widening the signature, since this address has a real
//            mangled name the linker's import table pins. The indirect
//            call at 0x0062E4C4 is __stdcall with the object as an
//            EXPLICIT first stack arg (not `this` in ecx - vtbl[13] takes
//            6 params), so no VCall shim is needed, just a plain
//            `__stdcall` function-pointer typedef. Landed at
//            mnemonic_similarity 0.789 (252 vs 278 bytes).

typedef int (__stdcall *Slot13Fn)(void *, void *, int, void *, int, int);

int Net::on_player_added(char * a1, unsigned int a2) {
    char *self = reinterpret_cast<char *>(this);

    if (*g_009be600 == 0) {
        return 7;
    }
    if (a2 == 0) {
        return 0x10;
    }

    unsigned char buf[0x50];
    memset(buf, 0, 0x50);
    *reinterpret_cast<int *>(buf) = 0x50;

    int *guidSrc = *reinterpret_cast<int **>(self + 0x6e8);
    *reinterpret_cast<int *>(buf + 0x18) = guidSrc[0];
    *reinterpret_cast<int *>(buf + 0x1c) = guidSrc[1];
    *reinterpret_cast<int *>(buf + 0x20) = guidSrc[2];
    *reinterpret_cast<int *>(buf + 0x24) = guidSrc[3];

    char *nameArg = 0;
    if (a1 != 0 && *a1 != 0) {
        nameArg = a1;
    }
    *reinterpret_cast<char **>(buf + 0x34) = nameArg;

    *g_009be4e8 = static_cast<int>(a2);

    void *obj = *reinterpret_cast<void **>(g_009be600);
    void **vtbl = *reinterpret_cast<void ***>(obj);
    Slot13Fn fn = reinterpret_cast<Slot13Fn>(vtbl[13]);
    int rv = fn(obj, buf, 0, reinterpret_cast<void *>(g_00630c30), 0, 1);

    *g_009be4e8 = 0;
    sub_5d49d0();

    if (rv != 0) {
        return 1;
    }

    char *player = reinterpret_cast<char *>(a2);
    if (*reinterpret_cast<int *>(player + 0x10) - 1 >= -1) {
        int limit = *reinterpret_cast<int *>(player + 0x10);
        *reinterpret_cast<int *>(player + 0xc) = *reinterpret_cast<int *>(player + 8);
        int n = abs(-1);
        if (n <= limit) {
            n = abs(-1);
            if (n > 0) {
                do {
                    n--;
                    *reinterpret_cast<int *>(player + 0xc) =
                        *reinterpret_cast<int *>(*reinterpret_cast<int *>(player + 0xc) + 0x10);
                } while (n != 0);
            }
            *reinterpret_cast<int *>(player + 0x14) = *reinterpret_cast<int *>(player + 0x10) - 1;
        }
    }
    return 0;
}
