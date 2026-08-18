// ORIGINAL: 0x005D2C0A sub_5d2c0a 0x005D2C0A-0x005D2D13
// RULED-OUT: BYTE_EXACT is structurally unreachable here without `__asm` (banned), same finding as the sibling 8-point butterfly at 0x005D2B20 (see src/unrecovered/005d2b20.cpp): the disassembly reads `esi` (src) from the very first real instruction and `edi` (dst, saved via `push edi` at entry and reloaded from the stack near the end) with no stack setup for either, which none of __cdecl/__stdcall/__fastcall/__thiscall can request. Landed as `sub_5d2c0a(int *dst, int *src)` (MISMATCH from instruction 0, the prologue) so the arithmetic is on record: identical sum/difference/`__int64` fixed-point-multiply butterfly to the sibling, but every combined pair is additionally scaled by `>> 17` before it is stored, which the sibling's version does not do.
// size      265 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

extern "C" void __cdecl sub_5d2c0a(int *dst, int *src) {
    int iVar9 = src[3] + src[5];
    int iVar8 = src[7] + src[1];
    float fVar2 = (float)(src[5] - src[3]);
    float fVar3 = (float)(src[1] - src[7]);
    float fVar1 = (fVar3 + fVar2) * *(float *)g_009c95a0;
    int iVar6 = (int)(((__int64)(iVar8 - iVar9) * (__int64)*g_009c9594) >> 32) * 2;
    int local_1c = (int)(double)(fVar2 * *(float *)g_009c9598 + fVar1 + *(float *)g_009c95a4);
    int local_14 = (int)(double)((fVar3 * *(float *)g_009c959c - fVar1) + *(float *)g_009c95a4);
    int iVar4 = local_1c + iVar6;
    iVar6 = iVar6 + local_14;
    local_14 = local_14 + iVar9 + iVar8;
    iVar9 = src[4] + src[0];
    int iVar5 = src[0] - src[4];
    int iVar7 = (int)(((__int64)(src[2] - src[6]) * (__int64)*g_009c9594) >> 32);
    iVar8 = iVar7 * 2;
    int iVar10 = src[6] + src[2] + iVar8;
    iVar8 = iVar8 + iVar5;
    iVar5 = iVar5 + iVar7 * -2;
    iVar7 = iVar10 + iVar9;
    iVar9 = iVar9 - iVar10;
    dst[2] = (iVar5 + iVar4) >> 17;
    dst[5] = (iVar5 - iVar4) >> 17;
    dst[3] = (iVar9 + local_1c) >> 17;
    dst[4] = (iVar9 - local_1c) >> 17;
    dst[0] = (iVar7 + local_14) >> 17;
    dst[1] = (iVar6 + iVar8) >> 17;
    dst[6] = (iVar8 - iVar6) >> 17;
    dst[7] = (iVar7 - local_14) >> 17;
}
