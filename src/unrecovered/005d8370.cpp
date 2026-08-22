// ORIGINAL: 0x005D8370 ?draw@Buffer@@QAEHPAVBuffer@@HHH@Z 0x005D8370-0x005D84B7
// TRIED: routing the slot-0x74/0x14/0x80 calls through the shared VCall shim (thiscall receiver) - the raw disassembly pushes the object pointer as an EXPLICIT stack argument (`push eax`) with no matching `mov ecx,...` setting a receiver, and ecx at call time already holds the vtable pointer itself rather than the object, so these are plain cdecl function-pointer calls that happen to take the object pointer as their own first argument, not virtual dispatch through an introduced class - matches Ghidra's own rendering exactly, which shows no separate `this`. Best reached: MISMATCH, edit_count first at #0 (prologue reserves the two locals via push instead of `sub esp,8`), 309 bytes vs 327, mnemonic similarity ~0.88.
// size      327 bytes
// prototype int (__thiscall ?draw@Buffer@@QAEHPAVBuffer@@HHH@Z)(Buffer* this, Buffer*, int, int horiz_pos, int vert_pos)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E4B9A
// indirect  0x005D83B2 0x005D83C7 0x005D8485
// CATALOGUE DEFECT: calling `((Sprite *)g_009b3a50)->draw(...)` through the
//            scaffold's own `Sprite` class scored BYTE_EXACT-shaped MISMATCH
//            in one run and C2065 'Sprite': undeclared identifier moments
//            later on an UNCHANGED body - the class disappeared from the
//            generated unit between two back-to-back invocations while
//            .opensmacx/byte-match.csv was mid-rewrite (5 minutes old at
//            the time). Routed through an introduced `SpriteShim` class
//            instead so this body does not depend on that scaffold slice.
class SpriteShim {
public:
    int draw(Buffer *, int, int, int);
}; int Buffer::draw(Buffer * a1, int a2, int a3, int a4) {
    typedef int (__cdecl *Slot29Fn)(void *, int, void *);
    typedef int (__cdecl *Slot5Fn)(void *, int, int, int, int, int);
    typedef int (__cdecl *Slot32Fn)(void *, int);
    char *base = (char *)this;
    char *abase = (char *)a1;
    void *piVar1;
    unsigned int local_8, local_4;
    int result;
    int vt;

    if (a1 == 0) {
        return 0x10;
    }
    piVar1 = *(void **)(base + 0x58);
    if (piVar1 != 0 && *(int *)(abase + 0x58) != 0) {
        local_8 = a2;
        local_4 = a2;
        vt = *(int *)piVar1;
        ((Slot29Fn)(*(int *)(vt + 0x74)))(piVar1, 8, &local_8);
        {
            void *recv2 = *(void **)(abase + 0x58);
            int vt2 = *(int *)recv2;
            ((Slot5Fn)(*(int *)(vt2 + 0x14)))(recv2, 0, *(int *)(base + 0x58), 0, 0x1008000, 0);
        }
        return 8;
    }
    if (piVar1 == 0) {
        unsigned int uVar2 = a2;
        int val54;
        if (a2 == (int)0xffffffff) {
            uVar2 = *(unsigned char *)g_00696d14;
        }
        val54 = *(int *)(base + 0x54);
        *g_009b3a54 = val54;
        *(int *)(base + 0x50) = val54;
        if (val54 != 0) {
            *(int *)(base + 0x6c) = *(int *)(base + 0x6c) + 1;
        }
        *g_009b3a5c = *(int *)(base + 0x4a8);
        *g_009b3a60 = *(int *)(base + 0x80);
        *g_009b3a64 = -*(int *)(base + 0x84);
        *g_009b3a68 = *(int *)(base + 0x80);
        *g_009b3a6c = -*(int *)(base + 0x84);
        *g_009b3a70 = 0;
        *g_009b3a74 = 0;
        *(unsigned char *)g_009b3a58 = (unsigned char)uVar2;
        *g_009b3a78 = 0;
        piVar1 = *(void **)(base + 0x58);
        if (piVar1 == 0) {
            int newcnt = *(int *)(base + 0x6c) - 1;
            *(int *)(base + 0x6c) = newcnt;
            if (0 < newcnt) {
                goto LAB;
            }
        } else {
            int newcnt = *(int *)(base + 0x6c) - 1;
            *(int *)(base + 0x6c) = newcnt;
            if (*(int *)(base + 0x50) == 0 || 0 < newcnt) {
                goto LAB;
            }
            {
                int vt3 = *(int *)piVar1;
                ((Slot32Fn)(*(int *)(vt3 + 0x80)))(piVar1, *(int *)(base + 0x50));
            }
        }
        *(int *)(base + 0x50) = 0;
        *(int *)(base + 0x6c) = 0;
    }
LAB:
    result = ((SpriteShim *)g_009b3a50)->draw(a1, a2, a3, a4);
    *g_009b3a54 = 0;
    return result;
}
