// ORIGINAL: 0x00638720 sub_638720 0x00638720-0x006388B2
// TRIED: chunked-record reader (tag/length header, three named chunk tags 0x1010000/0x1020000/0x1030000 plus a default skip through the indirect callback at g_009c0b90) - diverges at #3 with the loop-entry `lea` vs `push`, register/stack layout differs from here on; landed at 0.73 similarity.
// size      402 bytes
// prototype
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00638620 0x00638650 0x006391F0 0x00639390
// indirect  0x0063878D

typedef int (__cdecl *Sub638650Fn)(int, void *, int);
typedef int (__cdecl *Sub638620Fn)(int, void *);
typedef int (__cdecl *Sub6391f0Fn)(int, int);
typedef int (__cdecl *Sub639390Fn)(const char *);
typedef int (__cdecl *SkipFn)(int, int, int);

extern "C" int __cdecl sub_638720(int a1, int a2, int a3) {
    int remaining = a3 - 8;
    if (remaining == 0) {
        return 0;
    }

    char *base = reinterpret_cast<char *>(a2);

    do {
        struct { int tag; int length; } chunk;
        if ((reinterpret_cast<Sub638650Fn>(sub_638650))(a1, &chunk, 8) != 0) {
            return -1;
        }

        remaining -= chunk.length;
        if (remaining < 0) {
            const char *msg = reinterpret_cast<const char *>(*g_006989d0);
            (reinterpret_cast<Sub639390Fn>(sub_639390))(msg);
            return -1;
        }

        if (chunk.tag == 0x1010000) {
            int *out = reinterpret_cast<int *>(base + 0x10);
            if ((reinterpret_cast<Sub638620Fn>(sub_638620))(chunk.length - 8 + 1, out) != 0) {
                return -1;
            }
            if ((reinterpret_cast<Sub638650Fn>(sub_638650))(a1, reinterpret_cast<void *>(*out), chunk.length - 8) != 0) {
                return -1;
            }
            reinterpret_cast<char *>(*out)[chunk.length - 8] = 0;
        } else if (chunk.tag == 0x1020000) {
            int *out = reinterpret_cast<int *>(base + 0x14);
            if (*reinterpret_cast<float *>(base + 8) < *reinterpret_cast<float *>(g_00670fec)) {
                if ((reinterpret_cast<Sub638620Fn>(sub_638620))(0x1b02, out) != 0) {
                    return -1;
                }
                if ((reinterpret_cast<Sub6391f0Fn>(sub_6391f0))(a1, *out) != 0) {
                    return -1;
                }
            } else {
                if ((reinterpret_cast<Sub638620Fn>(sub_638620))(chunk.length - 8, out) != 0) {
                    return -1;
                }
                if ((reinterpret_cast<Sub638650Fn>(sub_638650))(a1, reinterpret_cast<void *>(*out), chunk.length - 8) != 0) {
                    return -1;
                }
            }
        } else if (chunk.tag == 0x1030000) {
            int *out = reinterpret_cast<int *>(base + 0x18);
            if ((reinterpret_cast<Sub638620Fn>(sub_638620))(chunk.length - 8, out) != 0) {
                return -1;
            }
            if ((reinterpret_cast<Sub638650Fn>(sub_638650))(a1, reinterpret_cast<void *>(*out), chunk.length - 8) != 0) {
                return -1;
            }
        } else {
            int skipped = (reinterpret_cast<SkipFn>(*g_009c0b90))(a1, chunk.length - 8, 1);
            if (skipped < 0) {
                const char *msg = reinterpret_cast<const char *>(*g_006989d0);
                (reinterpret_cast<Sub639390Fn>(sub_639390))(msg);
                return -1;
            }
        }
    } while (remaining != 0);

    return 0;
}
