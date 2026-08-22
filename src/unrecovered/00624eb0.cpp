// ORIGINAL: 0x00624EB0 sub_624eb0 0x00624EB0-0x00625075
// TRIED: the two fixed-point globals at 0x9BB490/0x9BB4F4 hold POINTERS (dereferenced then indexed *8), not plain scalars, so they are declared `int **const` and dereferenced once. Landing the direct transcription; the extra locals needed push the prologue to a spilled form the original's tighter register allocation does not use (divergence starts at instruction #0).
// size      453 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628AD0
// PROPOSAL: extern "C" int __cdecl sub_624eb0() ->
//           extern "C" int __cdecl sub_624eb0(int *param_1, int param_2,
//           unsigned char *param_3) - evidence: three stack reads at
//           [esp+0xc] (before any push), [esp+0x14] and [esp+0x1c] (after
//           the 4-register prologue), matching IDA's/Ghidra's 3-argument
//           guess and cdecl (no `ret N`).

static int *const g_009bb4b0 = (int *)0x009BB4B0;
static int *const g_009bb4b4 = (int *)0x009BB4B4;
static int **const g_009bb490 = (int **)0x009BB490;
static int **const g_009bb4f4 = (int **)0x009BB4F4;

struct Entry8 {
  int a;
  int b;
};

extern "C" int __cdecl sub_624eb0(int *param_1, int param_2, unsigned char *param_3) {
  int idx;
  int delta;
  int slotBase;
  int slotIndex;
  int range;

  if (param_2 == *g_009bb4b4) {
    return 0;
  }

  do {
    idx = param_1[0] + param_2;
    if (*g_009bb4b0 - 1 < idx) {
      idx = 0;
    } else if (idx < 0) {
      idx = *g_009bb4b0 - 1;
    }

    delta = reinterpret_cast<Entry8 *>(*g_009bb4f4)[idx].b -
            reinterpret_cast<Entry8 *>(*g_009bb4f4)[param_2].b;
    param_1[1] = delta;
    if (delta != 0) {
      if (delta < 1) {
        return 0;
      }

      slotIndex = *reinterpret_cast<int *>(param_3 + 4);
      slotBase = slotIndex + 1;
      *reinterpret_cast<int *>(param_3 + 4) = slotBase;
      slotIndex = slotIndex * 0x3c;
      if (slotBase == 10) {
        param_3[4] = 0;
        param_3[5] = 0;
        param_3[6] = 0;
        param_3[7] = 0;
      }

      param_1[2] = idx;
      range = param_1[1];
      param_1[3] = reinterpret_cast<Entry8 *>(*g_009bb490)[param_2].a << 0x10;
      param_1[4] = reinterpret_cast<Entry8 *>(*g_009bb490)[param_2].b << 0x10;
      if ((*param_3 & 2) == 0) {
        param_1[5] = *reinterpret_cast<int *>(param_3 + slotIndex + 0x1c);
        param_1[6] = *reinterpret_cast<int *>(param_3 + slotIndex + 0x20);
      } else {
        int fx = fixed_div(
            reinterpret_cast<Entry8 *>(*g_009bb490)[idx].a * 0x10000 - param_1[3],
            range << 0x10);
        *reinterpret_cast<int *>(param_3 + slotIndex + 0x1c) = fx;
        param_1[5] = fx;
        range = param_1[1];
        int fy = fixed_div(
            reinterpret_cast<Entry8 *>(*g_009bb490)[idx].b * 0x10000 - param_1[4],
            range << 0x10);
        *reinterpret_cast<int *>(param_3 + slotIndex + 0x20) = fy;
        param_1[6] = fy;
      }

      param_1[7] = reinterpret_cast<Entry8 *>(*g_009bb4f4)[param_2].a;
      idx = reinterpret_cast<Entry8 *>(*g_009bb4f4)[idx].a -
            reinterpret_cast<Entry8 *>(*g_009bb4f4)[param_2].a;
      if ((*param_3 & 2) != 0) {
        range = param_1[1];
        *reinterpret_cast<int *>(param_3 + slotIndex + 0x28) = idx / range;
        param_1[8] = idx / range;
        if (idx < 0) {
          idx = -idx;
          param_1[9] = -1;
          param_1[10] = 1 - param_1[1];
        } else {
          param_1[9] = 1;
          param_1[10] = 0;
        }
        range = param_1[1];
        *reinterpret_cast<int *>(param_3 + slotIndex + 0x34) = idx % range;
        param_1[0xb] = idx % range;
        param_1[0xc] = param_1[1];
        return 1;
      }

      param_1[8] = *reinterpret_cast<int *>(param_3 + slotIndex + 0x28);
      if (idx < 0) {
        param_1[9] = -1;
        param_1[10] = 1 - param_1[1];
      } else {
        param_1[9] = 1;
        param_1[10] = 0;
      }
      param_1[0xb] = *reinterpret_cast<int *>(param_3 + slotIndex + 0x34);
      param_1[0xc] = param_1[1];
      return 1;
    }

    param_2 = idx;
  } while (idx != *g_009bb4b4);

  return 0;
}
