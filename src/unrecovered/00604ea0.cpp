// ORIGINAL: 0x00604EA0 ?UNK2@BasePop@@QAEHXZ 0x00604EA0-0x0060500A
// RULED-OUT: nested `if (str==0) { str=fallback; if (str==0) str=literal; }` and the flattened two-`if` spelling both lower to inline `jne`/`mov` sequences here, where the original keeps a `je`/`push`/`jmp`-to-common-call-site shape for the same three-way string fallback (repeated twice, for the two caption strings). 0.76 mnemonic similarity, first divergence at #16 (that shape difference); not chased further. The `*(int*)4` read at self+0x215c's null-fallback branch (`xor eax,eax; mov eax,[eax+4]`) is reproduced literally via a fixed global at address 4, matching what Ghidra's `uRam00000004` names.
// size      362 bytes
// prototype int (__thiscall ?UNK2@BasePop@@QAEHXZ)(BasePop* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00619280 0x00644F3A

extern "C" int abs(int);

class Font;

static char **const g_009b8d80 = (char **)0x009B8D80;
static char **const g_009b8d84 = (char **)0x009B8D84;
static int *const g_00000004 = (int *)0x00000004;

class Font { public:
    int width(char *);
};

class BasePop { public:
    int UNK2();
};

int BasePop::UNK2() {
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int *>(self + 0x30b0) == 0) {
        return 0;
    }

    int maxWidth = 0;

    if (!(*reinterpret_cast<unsigned int *>(self + 0x30a8) & 0x10000)) {
        char *str = *reinterpret_cast<char **>(self + 0x3224);
        if (str == 0) {
            str = *g_009b8d80;
            if (str == 0) {
                str = reinterpret_cast<char *>(0x00697014);
            }
        }
        Font *font = *reinterpret_cast<Font **>(self + 0x316c);
        int w = font->width(str);
        if (w > 0) {
            maxWidth = w;
        }
    }

    if (*reinterpret_cast<unsigned char *>(self + 0x30a8) & 0x40) {
        char *str = *reinterpret_cast<char **>(self + 0x3228);
        if (str == 0) {
            str = *g_009b8d84;
            if (str == 0) {
                str = reinterpret_cast<char *>(0x00697018);
            }
        }
        Font *font = *reinterpret_cast<Font **>(self + 0x316c);
        int w = font->width(str);
        if (w > maxWidth) {
            maxWidth = w;
        }
    }

    int count = *reinterpret_cast<int *>(self + 0x2160);
    if (count - 1 >= -1) {
        *reinterpret_cast<void **>(self + 0x215c) = *reinterpret_cast<void **>(self + 0x2158);
        int n = abs(-1);
        if (n <= *reinterpret_cast<int *>(self + 0x2160)) {
            n = abs(-1);
            if (n > 0) {
                do {
                    --n;
                    void *node = *reinterpret_cast<void **>(self + 0x215c);
                    *reinterpret_cast<void **>(self + 0x215c) =
                        *reinterpret_cast<void **>(reinterpret_cast<char *>(node) + 0x10);
                } while (n != 0);
            }
            *reinterpret_cast<int *>(self + 0x2164) = *reinterpret_cast<int *>(self + 0x2160) - 1;
        }
    }

    int i = 0;
    int total = *reinterpret_cast<int *>(self + 0x2160);
    if (total > 0) {
        do {
            int has2158 = *reinterpret_cast<int *>(self + 0x2158);
            if (has2158 != 0) {
                void *node = *reinterpret_cast<void **>(self + 0x215c);
                void *next = *reinterpret_cast<void **>(reinterpret_cast<char *>(node) + 0xc);
                *reinterpret_cast<void **>(self + 0x215c) = next;
                int idx = *reinterpret_cast<int *>(self + 0x2164) + 1;
                if (idx == *reinterpret_cast<int *>(self + 0x2160)) {
                    idx = 0;
                }
                *reinterpret_cast<int *>(self + 0x2164) = idx;
            }

            char *strArg;
            void *node2 = *reinterpret_cast<void **>(self + 0x215c);
            if (node2 == 0) {
                strArg = 0;
            } else if (*reinterpret_cast<int *>(self + 0x2158) == 0) {
                strArg = reinterpret_cast<char *>(*g_00000004);
            } else {
                void *p8 = *reinterpret_cast<void **>(reinterpret_cast<char *>(node2) + 8);
                strArg = *reinterpret_cast<char **>(reinterpret_cast<char *>(p8) + 4);
            }

            Font *font = *reinterpret_cast<Font **>(self + 0x316c);
            int w = font->width(strArg);
            if (w > maxWidth) {
                maxWidth = w;
            }

            ++i;
        } while (i < *reinterpret_cast<int *>(self + 0x2160));
    }

    return maxWidth;
}
