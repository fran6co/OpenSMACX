// ORIGINAL: 0x00638CC0
// name      sub_638cc0
// size      276 bytes
// spans     0x00638CC0-0x00638DD4
// prototype
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00638620 0x00638650 0x00639390
// indirect  0x00638D1B
// RULED-OUT: only one source-form tried; the callee-save register
//            allocation (ebx/ebp/esi/edi across the whole loop) at the
//            prologue diverges early. Landing the closest structural
//            translation (mnemonic_similarity 0.635).

typedef int (__cdecl *Fn638650)(int, void *, int);
typedef int (__cdecl *Fn638620)(int, int *);
typedef void (__cdecl *Fn639390)(const char *);
typedef int (__cdecl *Fn9c0b90)(int, int, int);

extern "C" int __cdecl sub_638620();
extern "C" int __cdecl sub_638650();
extern "C" int __cdecl sub_639390();
static int *const g_006989d0 = (int *)0x006989D0;
static int *const g_009c0b90 = (int *)0x009C0B90;

extern "C" int __cdecl sub_638cc0(int a1, int *a2, int a3) {
    struct { int tag; int len; } buf;

    a3 -= 8;
    if (a3 == 0) {
        return 0;
    }
    for (;;) {
        int rv = reinterpret_cast<Fn638650>(&sub_638650)(a1, &buf, 8);
        if (rv != 0) {
            return -1;
        }
        a3 -= buf.len;
        int size = buf.len - 8;
        if (a3 < 0) {
            reinterpret_cast<Fn639390>(&sub_639390)(reinterpret_cast<const char *>(g_006989d0));
            return -1;
        }
        if (buf.tag == 0x4040201) {
            int *outPtr = a2 + 1;
            int r2;
            if (*a2 == 0) {
                r2 = reinterpret_cast<Fn638620>(&sub_638620)(size, outPtr);
            }
            else {
                r2 = reinterpret_cast<Fn638620>(&sub_638620)(*a2, outPtr);
            }
            if (r2 != 0) {
                return -1;
            }
            rv = reinterpret_cast<Fn638650>(&sub_638650)(a1, reinterpret_cast<void *>(*outPtr), size);
            if (rv != 0) {
                return -1;
            }
        }
        else if (buf.tag == 0x4040202) {
            rv = reinterpret_cast<Fn638650>(&sub_638650)(a1, a2, size);
            if (rv != 0) {
                return -1;
            }
        }
        else {
            int r3 = (*reinterpret_cast<Fn9c0b90 *>(g_009c0b90))(a1, size, 1);
            if (r3 < 0) {
                reinterpret_cast<Fn639390>(&sub_639390)(reinterpret_cast<const char *>(g_006989d0));
                return -1;
            }
        }
        if (a3 == 0) {
            return 0;
        }
    }
}
