// ORIGINAL: 0x00642350
// name      sub_642350
// size      295 bytes
// spans     0x00642350-0x00642477
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: no ecx receiver - all three arguments read from the stack
//            before any push, matching IDA's three-int guess, so a plain
//            `int a1, int a2, int a3` stays a free function over an
//            opaque record pointer. Looks like a Huffman/LZ statistics
//            update: records a (distance, length) or literal pair into a
//            growing history table at 0x1694, bumps one of three
//            frequency tables depending on whether the entry is a literal
//            (distance 0) or a match, then every 4096 entries estimates
//            whether the accumulated tree cost still fits the remaining
//            output budget and signals an early flush. The end-pointer
//            comparison in that scan (`piVar5 < g_00698f94`) uses the
//            scaffold's own fixed-address global rather than a literal, per
//            the addressing-does-work rule for a loop bound.

extern "C" int __cdecl sub_642350(int a1, int a2, int a3) {
  char *e = (char *)a1;

  *(short *)(*(int *)(e + 0x1698) + *(int *)(e + 0x1694) * 2) = (short)a2;
  *(char *)(*(int *)(e + 0x168c) + *(int *)(e + 0x1694)) = (char)a3;
  *(int *)(e + 0x1694) = *(int *)(e + 0x1694) + 1;

  if (a2 == 0) {
    short *ps = (short *)(e + 0x88 + a3 * 4);
    *ps = *ps + 1;
  } else {
    *(int *)(e + 0x16a8) = *(int *)(e + 0x16a8) + 1;
    unsigned char idxTable1 = *((unsigned char *)g_009beb1c + a3);
    short *ps = (short *)(e + 0x48c + (unsigned int)idxTable1 * 4);
    *ps = *ps + 1;

    unsigned char bVar2;
    if ((unsigned int)(a2 - 1) < 0x100) {
      bVar2 = *((unsigned char *)g_009bec90 - 1 + a2);
    } else {
      bVar2 = *((unsigned char *)g_009bed90 + (((unsigned int)(a2 - 1)) >> 7));
    }
    ps = (short *)(e + 0x97c + (unsigned int)bVar2 * 4);
    *ps = *ps + 1;
  }

  if (*(int *)(e + 0x78) > 2) {
    unsigned int uVar3 = *(unsigned int *)(e + 0x1694);
    if ((uVar3 & 0xfff) == 0) {
      unsigned int uVar7 = uVar3 * 8;
      int *piVar5 = g_00698f1c;
      unsigned short *puVar6 = (unsigned short *)(e + 0x97c);
      do {
        int iVar4 = *piVar5;
        piVar5 = piVar5 + 1;
        uVar7 = uVar7 + (unsigned int)(iVar4 + 5) * (unsigned int)*puVar6;
        puVar6 = puVar6 + 2;
      } while (piVar5 < g_00698f94);
      if ((*(unsigned int *)(e + 0x16a8) < (uVar3 >> 1)) &&
          (((uVar7 >> 2) & 0x3ffffffe) <
           (unsigned int)((*(int *)(e + 0x60) - *(int *)(e + 0x50)) &
                          0xfffffffe))) {
        return 1;
      }
    }
  }
  return *(int *)(e + 0x1694) == *(int *)(e + 0x1690) - 1;
}
