// ORIGINAL: 0x005D29E1 sub_5d29e1 0x005D29E1-0x005D2B20
// TRIED: this is not addressable with a normal calling convention - the body reads unaff_ESI/unaff_EDI with no stack access and no register setup of its own (first instruction is `push edi`, `mov ebx,[esi+0x14]` immediately after), so the 3 callers must be pre-loading esi/edi before `call`ing in. That is an interprocedural register-allocation artifact of compiling the 3 call sites and this body together as one translation unit; it cannot be reproduced by compiling this function alone under any ordinary C++ signature, since VC6 only picks a non-standard register ABI for an internal-linkage callee when it can see all of its callers. Declared it as a plain two-pointer __cdecl function instead (src, dst) for a faithful, compiling body. Also not reproduced: the fadd-then- reinterpret-low-dword pattern for double->int (`fstp qword ptr`, then `mov` of the low 4 bytes) is the classic magic- number float truncation trick that avoids calling `_ftol`; mimicked with a raw double-then-reinterpret-cast here. Best reached: MISMATCH, edit_count 7, 455 bytes vs 319 - the whole prologue/epilogue and call sequencing differ because of the calling-convention gap above.
// size      319 bytes
// prototype
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
extern "C" void __cdecl sub_5d29e1(int *src, int *dst) {
    int iVar2, iVar3, iVar4, iVar5, iVar6, iVar7, iVar8;
    double d1, d2;
    int local_1c, local_14;
    float fVar1;

    iVar7 = src[5];
    iVar6 = src[3];
    iVar5 = src[1];
    iVar3 = src[7];
    if (src[6] || iVar6 || iVar3 || src[2] || iVar7 || iVar5 || src[4]) {
        fVar1 = ((float)(iVar5 - iVar3) + (float)(iVar7 - iVar6)) * *(float *)g_009c95a0;
        iVar4 = (int)(((__int64)(iVar3 + iVar5 - iVar6 - iVar7) * *g_009c9594) >> 32) * 2;
        d1 = (double)((float)(iVar7 - iVar6) * *(float *)g_009c9598 + fVar1) + *(float *)g_009c95a4;
        local_1c = *(int *)&d1;
        d2 = (double)((float)(iVar5 - iVar3) * *(float *)g_009c959c - fVar1) + *(float *)g_009c95a4;
        local_14 = *(int *)&d2;
        iVar2 = local_1c + iVar4;
        iVar4 = iVar4 + local_14;
        local_14 = local_14 + iVar6 + iVar7 + iVar3 + iVar5;
        iVar7 = src[4] + src[0];
        iVar3 = src[0] - src[4];
        iVar5 = (int)(((__int64)(src[2] - src[6]) * *g_009c9594) >> 32);
        iVar6 = iVar5 * 2;
        iVar8 = src[6] + src[2] + iVar6;
        iVar6 = iVar6 + iVar3;
        iVar3 = iVar3 - iVar5 * 2;
        iVar5 = iVar8 + iVar7;
        iVar7 = iVar7 - iVar8;
        dst[0x12] = iVar3 + iVar2;
        dst[0x2d] = iVar3 - iVar2;
        dst[0x1b] = iVar7 + local_1c;
        dst[0x24] = iVar7 - local_1c;
        dst[0] = iVar5 + local_14;
        dst[9] = iVar4 + iVar6;
        dst[0x36] = iVar6 - iVar4;
        dst[0x3f] = iVar5 - local_14;
        return;
    }
    iVar7 = src[0];
    dst[0] = iVar7;
    dst[9] = iVar7;
    dst[0x12] = iVar7;
    dst[0x1b] = iVar7;
    dst[0x24] = iVar7;
    dst[0x2d] = iVar7;
    dst[0x36] = iVar7;
    dst[0x3f] = iVar7;
    return;
}
