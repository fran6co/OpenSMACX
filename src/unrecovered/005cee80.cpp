// ORIGINAL: 0x005CEE80 sub_5cee80 0x005CEE80-0x005CEF8A
// RULED-OUT: same unexpressible-convention finding as 0x005D2C0A and its sibling 0x005D2B20 - `esi` (src) is read from the first real instruction with no stack setup, so a normal parameter cannot reproduce the prologue (MISMATCH from instruction 0). Unlike 0x005D2C0A this one does NOT do the final `>> 17` scale (plain sum/difference like 0x005D2B20), and its `dst` comes from a global pointer (`g_009c3004`, a plain load) rather than a second implicit register, with the intermediate values spilled to fixed globals (`g_009c3008`.."14) instead of the stack - landed with a `src` parameter and the global dst load, which the lever note calls out as safe for a plain load.
// size      266 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

extern "C" void __cdecl sub_5cee80(int *src) {
    int *dst = *reinterpret_cast<int **>(g_009c3004);
    int iVar9 = src[3] + src[5];
    int iVar8 = src[7] + src[1];
    float fVar2 = (float)(src[5] - src[3]);
    float fVar3 = (float)(src[1] - src[7]);
    float fVar1 = (fVar3 + fVar2) * *(float *)g_009c3024;
    int iVar6 = (int)(((__int64)(iVar8 - iVar9) * (__int64)*g_009c3018) >> 32) * 2;
    int iStack8 = (int)(double)(fVar2 * *(float *)g_009c301c + fVar1 + *(float *)g_009c3028);
    int iStack10 = (int)(double)((fVar3 * *(float *)g_009c3020 - fVar1) + *(float *)g_009c3028);
    int iVar4 = iStack8 + iVar6;
    iVar6 = iVar6 + iStack10;
    iStack10 = iStack10 + iVar9 + iVar8;
    iVar9 = src[4] + src[0];
    int iVar5 = src[0] - src[4];
    int iVar7 = (int)(((__int64)(src[2] - src[6]) * (__int64)*g_009c3018) >> 32);
    iVar8 = iVar7 * 2;
    int iVar10 = src[6] + src[2] + iVar8;
    iVar8 = iVar8 + iVar5;
    iVar5 = iVar5 + iVar7 * -2;
    iVar7 = iVar10 + iVar9;
    iVar9 = iVar9 - iVar10;
    dst[2] = iVar5 + iVar4;
    dst[5] = iVar5 - iVar4;
    dst[3] = iVar9 + iStack8;
    dst[4] = iVar9 - iStack8;
    dst[0] = iVar7 + iStack10;
    dst[1] = iVar6 + iVar8;
    dst[6] = iVar8 - iVar6;
    dst[7] = iVar7 - iStack10;
}
