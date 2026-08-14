// ORIGINAL: 0x0063DFE0
// name      sub_63dfe0
// size      270 bytes
// spans     0x0063DFE0-0x0063E0EE
// prototype
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063DF40 0x0063DF90 0x00642BB0
// indirect  0x0063E043
// RULED-OUT: not chased to byte-exact - 86.4% mnemonic agreement (262 vs
//            270 bytes), diverges right after the validation checks. Landed
//            for the field layout: `param_1` is treated as an `int*` record
//            with a function-pointer "allocator" at index 8 (offset 0x20)
//            called cdecl as `(ctx, count, size)`, matching the disassembly
//            exactly; the allocated 0x18-byte record's sign/magnitude/shift
//            fields (indices 3/4/5) match Ghidra's `piVar1[3]`, `[4]`, `[5]`.

typedef void *(__cdecl *AllocFn)(void *, int, int);
typedef int (__cdecl *Fn1)(int);
typedef int (__cdecl *Fn3)(int, int, int);

extern "C" int __cdecl sub_63dfe0(int param_1, int param_2, char *param_3, int param_4) {
    if (param_3 == 0 || *param_3 != *reinterpret_cast<char *>(g_00697f28) || param_4 != 0x38) {
        return 0xfffffffa;
    }
    if (param_1 == 0) {
        return 0xfffffffe;
    }

    int *obj = reinterpret_cast<int *>(param_1);
    AllocFn alloc = reinterpret_cast<AllocFn>(obj[8]);
    obj[6] = 0;
    if (alloc == 0) {
        alloc = reinterpret_cast<AllocFn>(g_00640a50);
        obj[8] = 0;
        obj[10] = 0;
    }
    if (obj[9] == 0) {
        obj[9] = reinterpret_cast<int>(g_00640a70);
    }
    void *allocated = alloc(reinterpret_cast<void *>(obj[10]), 1, 0x18);
    obj[7] = reinterpret_cast<int>(allocated);
    if (allocated == 0) {
        return 0xfffffffc;
    }

    int *rec = reinterpret_cast<int *>(allocated);
    rec[5] = 0;
    rec[3] = 0;
    int magnitude = param_2;
    if (magnitude < 0) {
        magnitude = -magnitude;
        rec[3] = 1;
    }
    if (magnitude >= 8 && magnitude <= 0xf) {
        rec[4] = magnitude;
        int shifted = 1 << magnitude;
        int maskedAddr = rec[3] != 0 ? 0 : reinterpret_cast<int>(g_00640a80);
        int result = reinterpret_cast<Fn3>(sub_642bb0)(param_1, maskedAddr, shifted);
        rec[5] = result;
        if (rec[5] == 0) {
            reinterpret_cast<Fn1>(sub_63df90)(param_1);
            return 0xfffffffc;
        }
        reinterpret_cast<Fn1>(sub_63df40)(param_1);
        return 0;
    }
    reinterpret_cast<Fn1>(sub_63df90)(param_1);
    return 0xfffffffe;
}
