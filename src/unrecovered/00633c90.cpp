// ORIGINAL: 0x00633C90 sub_633c90 0x00633C90-0x00633D81 FILE
// RULED-OUT: nothing yet - MISMATCH #1 push/mov (66% mnemonic sim), extra local `int *self`/`void *cs` shift the prologue; not chased further.
// PROPOSAL: sub_633c90(void*, int, int, int) -> a __thiscall member (this =
//           receiver, `mov ebp, ecx` at entry, `ret 0x10` pops exactly the
//           4 explicit stack args) taking (int a2, int a3, int a4, int *a5);
//           evidence: [ecx+...] field reads with no matching stack slot for
//           a first parameter.
// size      241 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2 0x00645930
// indirect  0x00633C9F 0x00633CAD 0x00633CDE 0x00633CF3 0x00633D71
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x00633C90

typedef void (__stdcall *CritSectionFn)(void *);

class Sub633c90Host {
 public:
    int remove_entry(int a2, int a3, int a4, int *a5);
};

extern "C" void free(void *);
extern "C" void *memcpy(void *, const void *, unsigned int);

int Sub633c90Host::remove_entry(int a2, int a3, int a4, int *a5) {
    int *self = (int *)this;
    void *cs = (void *)((char *)self + 0xC);
    (*(CritSectionFn *)g_0066917c)(cs);
    int *node = (int *)self[0];
    if (node == 0) {
        (*(CritSectionFn *)g_00669174)(cs);
        return 0;
    }
    int *prev = 0;
    while (node[1] != a3 || node[0] != a4) {
        prev = node;
        node = (int *)node[5];
        if (node == 0) {
            (*(CritSectionFn *)g_00669174)(cs);
            return 0;
        }
    }
    if (a5 != 0) {
        *a5 = node[4];
    }
    if (node[3] != 0) {
        if (a2 != 0) {
            memcpy((void *)a2, (void *)node[3], node[4]);
        }
        if (node[3] != 0) {
            free((void *)node[3]);
        }
        node[3] = 0;
    }
    int result = node[2];
    if (prev == 0) {
        self[0] = *(int *)(self[0] + 0x14);
    } else {
        prev[5] = node[5];
    }
    free((void *)node);
    self[2] = self[2] - 1;
    (*(CritSectionFn *)g_00669174)(cs);
    return result;
}
