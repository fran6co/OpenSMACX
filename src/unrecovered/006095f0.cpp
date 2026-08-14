// ORIGINAL: 0x006095F0
// name      ?init@Dialog@@QAEHH@Z
// size      320 bytes
// spans     0x006095F0-0x00609730
// prototype int (__thiscall ?init@Dialog@@QAEHH@Z)(Dialog* this, int)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4620 0x00608F50
// indirect  0x00609655 0x00609667 0x00609683 0x006096D9 0x006096ED 0x00609708
// RULED-OUT: the scaffold-provided nullary `VCall` shim's slots cannot be
//            re-parameterised from the body (C2660 at `slot001(piVar1)`),
//            so declared an own `DialogVCall` class instead, permitted by
//            the "class to express __thiscall" carve-out. The two "delete
//            entry" calls are genuine virtual dispatch too, not a raw
//            function-pointer call: `ecx` already holds the computed
//            address (vtable[1] + object) from the preceding instructions
//            when `call dword ptr [edx]` executes, so it is
//            `((DialogVCall*)addr)->slot0(1)`, not a bare cdecl call.
//            Testing a1 == 0 first vs testing a1 != 0 first was tried; the
//            latter (a1 != 0 as the fallthrough) scores higher similarity
//            overall despite an earlier first divergence.
//            Best reached: MISMATCH, edit_count 31, 421 bytes vs 320,
//            mnemonic similarity ~0.79.
int Dialog::init(int a1) {
    class DialogVCall {
    public:
        virtual void slot0(int) = 0;
        virtual void slot1(void *) = 0;
    };
    char *base = (char *)this;
    void *piVar1;
    int iVar2;
    int a_addr;

    close();
    *(int *)(base + 0x1c) = a1;
    if (a1 != 0) {
        iVar2 = ((Heap *)(base + 4))->init(a1);
        if (iVar2 == 0) {
            if (*(int *)(base + 0xc4) != 0) {
                iVar2 = 0;
                if (0 < *(int *)(base + 0xcc)) {
                    do {
                        *(int *)(base + 200) = *(int *)(*(int *)(base + 0xc4) + 0xc);
                        piVar1 = *(void **)(*(int *)(base + 0xc4) + 8);
                        ((DialogVCall *)(base + 0xbc))->slot1(piVar1);
                        if (piVar1 != 0) {
                            a_addr = *(int *)(*(int *)piVar1 + 4) + (int)piVar1;
                            ((DialogVCall *)a_addr)->slot0(1);
                        }
                        *(int *)(*(int *)(base + 0xc4) + 8) = 0;
                        piVar1 = *(void **)(base + 0xc4);
                        if (piVar1 != 0) {
                            a_addr = *(int *)(*(int *)piVar1 + 4) + (int)piVar1;
                            ((DialogVCall *)a_addr)->slot0(1);
                        }
                        iVar2 = iVar2 + 1;
                        *(int *)(base + 0xc4) = *(int *)(base + 200);
                    } while (iVar2 < *(int *)(base + 0xcc));
                }
                *(int *)(base + 0xc4) = 0;
                *(int *)(base + 0xd0) = 0;
                *(int *)(base + 0xcc) = 0;
            }
            *(int *)(base + 0xd4) = (int)(base + 4);
            *(int *)(base + 0xd0) = 0;
            return 0;
        }
        return 4;
    }
    if (*(int *)(base + 0xc4) != 0) {
        iVar2 = 0;
        if (0 < *(int *)(base + 0xcc)) {
            do {
                *(int *)(base + 200) = *(int *)(*(int *)(base + 0xc4) + 0xc);
                piVar1 = *(void **)(*(int *)(base + 0xc4) + 8);
                ((DialogVCall *)(base + 0xbc))->slot1(piVar1);
                if (piVar1 != 0) {
                    a_addr = *(int *)(*(int *)piVar1 + 4) + (int)piVar1;
                    ((DialogVCall *)a_addr)->slot0(1);
                }
                *(int *)(*(int *)(base + 0xc4) + 8) = 0;
                piVar1 = *(void **)(base + 0xc4);
                if (piVar1 != 0) {
                    a_addr = *(int *)(*(int *)piVar1 + 4) + (int)piVar1;
                    ((DialogVCall *)a_addr)->slot0(1);
                }
                iVar2 = iVar2 + 1;
                *(int *)(base + 0xc4) = *(int *)(base + 200);
            } while (iVar2 < *(int *)(base + 0xcc));
        }
        *(int *)(base + 0xc4) = 0;
        *(int *)(base + 0xd0) = 0;
        *(int *)(base + 0xcc) = 0;
    }
    *(int *)(base + 0xd0) = 0;
    *(int *)(base + 0xd4) = 0;
    return 0;
}
