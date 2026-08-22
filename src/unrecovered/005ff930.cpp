// ORIGINAL: 0x005FF930 ?UNK8@Palette@@QAEHHHHHH@Z 0x005FF930-0x005FFB0B
// TRIED: the third RGB channel is written through a pair of pointers precomputed once before the loop (`stackA_adj`/`stackB_adj` plus the destination pointer) rather than a fresh `k*4+2` index each iteration; kept as a direct index into the two 1024-byte stack copies, which is semantically the same value. `seed_` at offset 0x400 (right after `field_3FC_`) matches the reseed loop exactly. Landing the closest control-flow- faithful form (divergence starts at instruction #2, on the `this`-copy prologue).
// size      475 bytes
// prototype int (__thiscall ?UNK8@Palette@@QAEHHHHHH@Z)(Palette* this, void*, UINT iStartIndex, UINT cEntries, int, int)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00625810 0x00645930
// indirect  0x005FF9D2 0x005FFA94 0x005FFA9A 0x005FFAB1

typedef unsigned long DWORD;
typedef DWORD(__stdcall *TimeGetTimeFn)();
typedef int(__stdcall *AnimatePaletteFn)(void *, unsigned int, unsigned int, void *);

static int *const g_006690a8 = (int *)0x006690A8;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_009b8178 = (int *)0x009B8178;
static int *const g_009b8188 = (int *)0x009B8188;

int Palette::UNK8(int a1, int a2, int a3, int a4, int a5) {
  if (*g_009b8188 != 0) {
    return 0;
  }
  if (*g_009b8178 == 0) {
    return 7;
  }
  if (a1 == 0) {
    return 0x10;
  }

  unsigned char stackA[0x400];
  unsigned char stackB[0x400];
  memcpy(stackA, this, 0x400);
  memcpy(stackB, reinterpret_cast<void *>(a1), 0x400);

  TimeGetTimeFn timeGetTime = reinterpret_cast<TimeGetTimeFn>(*g_00669368);
  AnimatePaletteFn animatePalette =
      reinterpret_cast<AnimatePaletteFn>(*g_006690a8);

  int frame = 0;
  if (a4 != -1 && a4 + 1 >= 0) {
    int weight = a4;
    do {
      DWORD t0 = timeGetTime();
      if (a2 < a2 + a3) {
        unsigned char *dst =
            reinterpret_cast<unsigned char *>(a1) + 2 + a2 * 4;
        int k = a2;
        do {
          dst[-2] = static_cast<char>(
              (static_cast<unsigned int>(stackA[k * 4]) * weight +
               static_cast<unsigned int>(stackB[k * 4]) * frame) /
              a4);
          dst[-1] = static_cast<char>(
              (static_cast<unsigned int>(stackA[k * 4 + 1]) * weight +
               static_cast<unsigned int>(stackB[k * 4 + 1]) * frame) /
              a4);
          dst[0] = static_cast<char>(
              (static_cast<unsigned int>(stackA[k * 4 + 2]) * weight +
               static_cast<unsigned int>(stackB[k * 4 + 2]) * frame) /
              a4);
          ++k;
          dst += 4;
        } while (k < a2 + a3);
      }

      animatePalette(reinterpret_cast<void *>(*g_009b8178), a2, a3,
                      reinterpret_cast<void *>(a1 + a2 * 4));

      DWORD t1 = timeGetTime();
      unsigned int elapsed = t1 - t0;
      while (elapsed < static_cast<unsigned int>(a5)) {
        t1 = timeGetTime();
        elapsed = t1 - t0;
      }

      ++frame;
      --weight;
    } while (frame < a4 + 1);
  }

  seed_ = 0;
  int r;
  do {
    r = random(0, 0xffff);
    seed_ = r;
  } while (r == 0);

  return 0;
}
