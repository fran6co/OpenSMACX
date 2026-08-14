// ORIGINAL: 0x005D427C
// name      sub_5d427c
// size      291 bytes
// spans     0x005D427C-0x005D439F
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: entry uses pushal/popal (all eight registers saved and
//            restored) around the whole body with cld and only rep movsb/
//            rep movsd inside, which VC6 never emits for ordinary compiled
//            C++ - this is very likely inline assembly in the original
//            source, which the rules here bar reproducing. Transcribed
//            Ghidra's decompilation (already a faithful read of the byte
//            stream) into plain C++ instead: a byte-oriented decoder that
//            walks a control word one bit at a time, alternating literal
//            dword/byte copies from the source with back-reference copies
//            from already-written output, matching an LZ-style unpacker.
//            No `__asm`; this compiles but will not reach the original's
//            register-saving shape.

extern "C" int __cdecl sub_5d427c(int a1, int a2, int a3) {
  unsigned short *param_3 = (unsigned short *)a1;
  unsigned char *param_4 = (unsigned char *)a2;
  int param_5 = a3;

  unsigned int uVar4;
  unsigned int uVar2;
  int iVar3;
  signed char cVar5;
  unsigned char bVar6;
  unsigned char uVar1;
  unsigned char *puVar8;
  unsigned char *puVar9;

  if (param_3 != 0) {
    unsigned char *puVar7 = (unsigned char *)(param_3 + 1);
    if (*param_3 & 1) {
      puVar7 = (unsigned char *)param_3 + 5;
    }
    puVar8 = puVar7 + 3;
    if (param_5 != 0) {
      do {
        uVar4 = *(unsigned int *)puVar8;
        cVar5 = (signed char)uVar4;
        while (uVar2 = uVar4 & 0xff, cVar5 < 0) {
          for (;;) {
            bVar6 = (unsigned char)(uVar4 >> 8);
            uVar1 = (unsigned char)(uVar4 >> 0x10);
            if ((signed char)((signed char)uVar2 * 2) >= 0) {
              break;
            }
            if ((signed char)((signed char)uVar2 << 2) < 0) {
              if ((unsigned char)uVar4 > 0xfb) {
                unsigned int n = uVar4 & 3;
                puVar8 = puVar8 + 1;
                while (n != 0) {
                  *param_4 = *puVar8;
                  param_4 = param_4 + 1;
                  puVar8 = puVar8 + 1;
                  n = n - 1;
                }
                goto label_5d4352;
              }
              puVar8 = puVar8 + 1;
              for (iVar3 = (int)(uVar4 & 0x1f) + 1; iVar3 != 0; iVar3 = iVar3 - 1) {
                *(unsigned int *)param_4 = *(unsigned int *)puVar8;
                puVar8 = puVar8 + 4;
                param_4 = param_4 + 4;
              }
              uVar4 = *(unsigned int *)puVar8;
              cVar5 = (signed char)uVar4;
            } else {
              puVar8 = puVar8 + 1;
              for (uVar2 = uVar4 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
                *param_4 = *puVar8;
                puVar8 = puVar8 + 1;
                param_4 = param_4 + 1;
              }
              {
                unsigned int bit4 = (uVar4 & 0x10) >> 4;
                unsigned int dist16 = (unsigned int)((bVar6 << 8) | uVar1);
                unsigned int distance = (bit4 << 0x10) | dist16;
                puVar9 = (param_4 - 1) - distance;
              }
              iVar3 = (int)((((unsigned char)uVar4 >> 2) << 8) |
                             (unsigned char)(uVar4 >> 0x18)) &
                          0x3ff;
              iVar3 = iVar3 + 5;
              for (; iVar3 != 0; iVar3 = iVar3 - 1) {
                *param_4 = *puVar9;
                puVar9 = puVar9 + 1;
                param_4 = param_4 + 1;
              }
              uVar4 = *(unsigned int *)puVar8;
              cVar5 = (signed char)uVar4;
            }
            uVar2 = uVar4 & 0xff;
            if (cVar5 >= 0) {
              goto label_5d42cc;
            }
          }
          puVar8 = puVar8 + 3;
          for (uVar2 = (unsigned int)(bVar6 >> 6); uVar2 != 0; uVar2 = uVar2 - 1) {
            *param_4 = *puVar8;
            puVar8 = puVar8 + 1;
            param_4 = param_4 + 1;
          }
          {
            unsigned int concat16 = (unsigned int)((bVar6 << 8) | uVar1);
            unsigned int distance = concat16 & 0xffff3fff;
            puVar9 = (param_4 - 1) - distance;
          }
          for (iVar3 = (int)(uVar4 & 0x3f) + 4; iVar3 != 0; iVar3 = iVar3 - 1) {
            *param_4 = *puVar9;
            puVar9 = puVar9 + 1;
            param_4 = param_4 + 1;
          }
          uVar4 = *(unsigned int *)puVar8;
          cVar5 = (signed char)uVar4;
        }
      label_5d42cc:
        puVar8 = puVar8 + 2;
        for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
          *param_4 = *puVar8;
          puVar8 = puVar8 + 1;
          param_4 = param_4 + 1;
        }
        {
          unsigned int concat16 =
              (unsigned int)((((unsigned char)uVar4 >> 5) << 8) |
                             (unsigned char)(uVar4 >> 8));
          puVar9 = (param_4 - 1) - concat16;
        }
        for (iVar3 = (int)((uVar4 & 0x1c) >> 2) + 3; iVar3 != 0; iVar3 = iVar3 - 1) {
          *param_4 = *puVar9;
          puVar9 = puVar9 + 1;
          param_4 = param_4 + 1;
        }
      } while (true);
    }
  }
label_5d4352:
  return 0;
}
