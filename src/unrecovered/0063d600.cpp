// ORIGINAL: 0x0063D600 FILE
// RULED-OUT: nothing yet - MISMATCH #1 mov/push (74% mnemonic sim). Register
//            pre-load order (param_2 into a saved reg before param_1) shifts
//            the prologue; not chased further. LEVER: calling the arity-0
//            scaffold declarations of the three unnamed callees through a
//            reinterpret-cast function-pointer typedef compiles and passes
//            real arguments, where calling them directly is locked to 0 args.
// name      sub_63d600
// size      239 bytes
// spans     0x0063D600-0x0063D6EF
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063D450 0x0063D6F0 0x00641280
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x0063D600

typedef int (__cdecl *TrFlushBlockFn)(int *, int, int, int);
typedef int (__cdecl *Sub63d450Fn)(int);
typedef int (__cdecl *Sub63d6f0Fn)(int *);

extern "C" int __cdecl sub_63d600(int *param_1, int param_2) {
    int iVar1;
    int iVar2;
    unsigned int uVar3;

    do {
        do {
            uVar3 = param_1[0x1a];
            if (uVar3 < 2) {
                ((Sub63d6f0Fn)sub_63d6f0)(param_1);
                uVar3 = param_1[0x1a];
                if (uVar3 == 0) {
                    if (param_2 == 0) {
                        return 1;
                    }
                    iVar1 = param_1[0x14];
                    if (iVar1 < 0) {
                        iVar2 = 0;
                    } else {
                        iVar2 = param_1[0xb] + iVar1;
                    }
                    ((TrFlushBlockFn)_tr_flush_block)(param_1, iVar2, param_1[0x18] - iVar1, param_2 == 4);
                    param_1[0x14] = param_1[0x18];
                    ((Sub63d450Fn)sub_63d450)(*param_1);
                    return *(int *)(*param_1 + 0x10) == 0;
                }
            }
            param_1[0x1a] = 0;
            uVar3 = param_1[0x18] + uVar3;
            param_1[0x18] = uVar3;
            if ((uVar3 == 0) || (0xfffe < uVar3)) {
                param_1[0x18] = 0xffff;
                param_1[0x1a] = uVar3 - 0xffff;
            }
            iVar1 = param_1[0x14];
        } while ((unsigned int)(param_1[0x18] - iVar1) < (unsigned int)param_1[8] - 0x106U);
        if (iVar1 < 0) {
            iVar2 = 0;
        } else {
            iVar2 = param_1[0xb] + iVar1;
        }
        ((TrFlushBlockFn)_tr_flush_block)(param_1, iVar2, param_1[0x18] - iVar1, 0);
        param_1[0x14] = param_1[0x18];
        ((Sub63d450Fn)sub_63d450)(*param_1);
    } while (*(int *)(*param_1 + 0x10) != 0);
    return 1;
}
