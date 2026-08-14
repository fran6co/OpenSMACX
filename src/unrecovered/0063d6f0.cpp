// ORIGINAL: 0x0063D6F0
// name      sub_63d6f0
// size      290 bytes
// spans     0x0063D6F0-0x0063D812
// prototype
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063D820
// RULED-OUT: no ecx receiver - the first stack slot at entry is the sole
//            argument, matching IDA's one-argument guess exactly, so this
//            stays a plain int* over the parameter. This looks like a
//            zlib-style sliding-window deflate fill: fields at 0x18/0x1a/
//            0xc read as strstart/lookahead/window_size, 0xb/0xe/0xd as
//            window/head/prev base pointers, 0x10/0x14/0x19 as further
//            counters, shifted down by the lookahead whenever the window
//            fills to within 0x106 bytes of its end. The one callee
//            (sub_63d820) is declared nullary by the scaffold; calling it
//            through a cast function pointer compiles but turns the
//            original's direct call into an indirect one at that site.

typedef int (__cdecl *Fn3_63d820)(int, int, int);

extern "C" int __cdecl sub_63d6f0(int a1) {
  Fn3_63d820 decode = (Fn3_63d820)&sub_63d820;
  int *param_1 = (int *)a1;
  unsigned int uVar1 = param_1[8];
  unsigned int uVar2, uVar5, uVar6, uVar7;
  int iVar3;
  unsigned short *puVar4;
  unsigned int *puVar8, *puVar9;

  do {
    uVar5 = param_1[0x18];
    uVar6 = (param_1[0xc] - uVar5) - param_1[0x1a];
    if (uVar6 == 0) {
      if ((uVar5 != 0) || (uVar7 = uVar1, param_1[0x1a] != 0)) {
      LAB_0063d725:
        uVar7 = uVar6;
        if ((unsigned int)(param_1[8] + -0x106 + uVar1) <= uVar5) {
          puVar8 = (unsigned int *)(param_1[0xb] + uVar1);
          puVar9 = (unsigned int *)param_1[0xb];
          for (uVar5 = uVar1 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
            *puVar9 = *puVar8;
            puVar8 = puVar8 + 1;
            puVar9 = puVar9 + 1;
          }
          for (uVar5 = uVar1 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
            *(unsigned char *)puVar9 = *(unsigned char *)puVar8;
            puVar8 = (unsigned int *)((int)puVar8 + 1);
            puVar9 = (unsigned int *)((int)puVar9 + 1);
          }
          param_1[0x19] = param_1[0x19] - uVar1;
          iVar3 = param_1[0x10];
          param_1[0x18] = param_1[0x18] - uVar1;
          param_1[0x14] = param_1[0x14] - uVar1;
          puVar4 = (unsigned short *)(param_1[0xe] + iVar3 * 2);
          do {
            puVar4 = puVar4 - 1;
            if (*puVar4 < uVar1) {
              uVar2 = 0;
            } else {
              uVar2 = *puVar4 - (unsigned short)uVar1;
            }
            iVar3 = iVar3 - 1;
            *puVar4 = (unsigned short)uVar2;
          } while (iVar3 != 0);
          puVar4 = (unsigned short *)(param_1[0xd] + uVar1 * 2);
          uVar5 = uVar1;
          do {
            puVar4 = puVar4 - 1;
            if (*puVar4 < uVar1) {
              uVar2 = 0;
            } else {
              uVar2 = *puVar4 - (unsigned short)uVar1;
            }
            uVar5 = uVar5 - 1;
            *puVar4 = (unsigned short)uVar2;
          } while (uVar5 != 0);
          uVar7 = uVar6 + uVar1;
        }
      }
    } else {
      if (uVar6 != 0xffffffff) {
        goto LAB_0063d725;
      }
      uVar7 = 0xfffffffe;
    }
    if (*(int *)(*param_1 + 4) == 0) {
      return 0;
    }
    iVar3 =
        decode(*param_1, param_1[0x1a] + param_1[0x18] + param_1[0xb], uVar7);
    uVar5 = param_1[0x1a] + iVar3;
    param_1[0x1a] = uVar5;
    if (2 < (int)uVar5) {
      uVar6 = (unsigned int)*(unsigned char *)(param_1[0x18] + param_1[0xb]);
      param_1[0xf] = uVar6;
      param_1[0xf] =
          ((uVar6 << ((unsigned char)param_1[0x13] & 0x1f)) ^
           (unsigned int)((unsigned char *)(param_1[0x18] + param_1[0xb]))[1]) &
          param_1[0x12];
    }
    if (0x105 < (int)uVar5) {
      return 0;
    }
    if (*(int *)(*param_1 + 4) == 0) {
      return 0;
    }
  } while (true);
}
