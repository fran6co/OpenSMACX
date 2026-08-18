// ORIGINAL: 0x00624D80 ?setup_edge@Texture@@QAAHPAUEdgeScan@@H@Z 0x00624D80-0x00624EA9
// RULED-OUT: the trailing division/remainder pair (offsets 0x2c/0x30) is computed with a THIRD separate idiv after the sign-dependent branch that already computed a quotient - the original never reuses that quotient's remainder, it redoes `diff % iVar1` from scratch, so the two writes here are kept as two separate statements rather than one combined div/mod to match that shape. `<< 0x10` kept as a shift rather than `* 0x10000` to match the original's shl. Landing the closest MISMATCH; divergence starts at instruction #2, a `mov` in the original reloading DAT_9bb4b0 versus a `push` in the rebuilt.
// size      297 bytes
// prototype
// callers   9   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628AD0

int __cdecl Texture::setup_edge(EdgeScan *a1, int a2) {
  char *e = (char *)a1;
  if (a2 == *g_009bb4b4) {
    return 0;
  }
  int base0 = *(int *)a1;
  for (;;) {
    int iVar3 = base0 + a2;
    if (*g_009bb4b0 - 1 < iVar3) {
      iVar3 = 0;
    } else if (iVar3 < 0) {
      iVar3 = *g_009bb4b0 - 1;
    }
    int *arr4f4 = *(int **)g_009bb4f4;
    int iVar1 = arr4f4[iVar3 * 2 + 1] - arr4f4[a2 * 2 + 1];
    *(int *)(e + 4) = iVar1;
    if (iVar1 != 0) {
      if (iVar1 <= 0) {
        return 0;
      }
      *(int *)(e + 8) = iVar3;
      int *arr490 = *(int **)g_009bb490;
      int scaled = iVar1 << 0x10;
      *(int *)(e + 0xc) = arr490[a2 * 2] << 0x10;
      *(int *)(e + 0x10) = arr490[a2 * 2 + 1] << 0x10;
      *(int *)(e + 0x14) =
          fixed_div((arr490[iVar3 * 2] << 0x10) - *(int *)(e + 0xc), scaled);
      *(int *)(e + 0x18) = fixed_div(
          (arr490[iVar3 * 2 + 1] << 0x10) - *(int *)(e + 0x10), scaled);
      *(int *)(e + 0x1c) = arr4f4[a2 * 2];
      int diff = arr4f4[iVar3 * 2] - arr4f4[a2 * 2];
      int quotient;
      if (diff < 0) {
        diff = -diff;
        *(int *)(e + 0x28) = 1 - *(int *)(e + 4);
        *(int *)(e + 0x24) = -1;
        quotient = -(diff / *(int *)(e + 4));
      } else {
        *(int *)(e + 0x24) = 1;
        quotient = diff / *(int *)(e + 4);
        *(int *)(e + 0x28) = 0;
      }
      *(int *)(e + 0x20) = quotient;
      *(int *)(e + 0x30) = *(int *)(e + 4);
      *(int *)(e + 0x2c) = diff % *(int *)(e + 4);
      return 1;
    }
    a2 = iVar3;
    if (iVar3 == *g_009bb4b4) {
      return 0;
    }
  }
}
