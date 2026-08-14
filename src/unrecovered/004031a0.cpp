// ORIGINAL: 0x004031A0 FILE
// RULED-OUT: nothing yet - MISMATCH #0 push/sub (79% mnemonic sim), stack
//            frame layout differs (compiler chose different spill slots
//            for the six loop temporaries); not chased further.
// name      ?compute@AlphaMenu@@QAEXXZ
// size      242 bytes
// spans     0x004031A0-0x00403292
// prototype void (__thiscall ?compute@AlphaMenu@@QAEXXZ)(AlphaMenu* this)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FA8A0 0x005FA960
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x004031A0

void AlphaMenu::compute() {
    char *const self = (char *)this;
    int stride = *(int *)(self + 0x1134);
    Spot *spot = (Spot *)(self + 0xA2C);
    spot->init(*(int *)(self + 0xA18));

    int iVar3 = *(int *)(self + 0x1108);
    int uVar2 = *(int *)(self + 0x1130);
    int local_14 = iVar3 + stride;
    int iVar4 = (*(int *)(self + 0xA18) - 1) * stride + iVar3;
    int local_24 = iVar4 + stride;

    if (*(int *)(self + 0xA18) > 0) {
        unsigned char *entry = (unsigned char *)(self + 0xA40);
        int local_1c = iVar3;
        int i = 0;
        do {
            if ((*entry & 1) != 0) {
                if ((*entry & 4) == 0) {
                    spot->add(i, 0, 0, iVar3, uVar2, local_14 - iVar3);
                    iVar3 = local_1c + stride;
                    local_14 = local_14 + stride;
                    local_1c = iVar3;
                } else {
                    spot->add(i, 0, 0, iVar4, uVar2, local_24 - iVar4);
                    iVar4 = iVar4 - stride;
                    local_24 = local_24 - stride;
                    iVar3 = local_1c;
                }
            }
            ++i;
            entry += 0x14;
        } while (i < *(int *)(self + 0xA18));
    }
}
