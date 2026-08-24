// ORIGINAL: 0x005F4D90 sub_5f4d90 0x005F4D90-0x005F4EB1
// TRIED: reads at offset 0xa8 (matches Win::poWinBase_) and 0x13c/0x140/ 0x144/0x148 (Win::outer_rect_) with the receiver cached to edi and no matching stack slot for it mean __thiscall on Win, but the class introduced to carry the method - needed only to avoid clashing with the scaffold's own closed Win member list - draws the same region-truncation the other multi-type units here do: a second top-level type ahead of an out-of-line definition returns the brace-depth to zero at its own close, before the real body, and this file's extractor never sees past it. Landed as a free stdcall function taking the receiver explicitly. The two indirect calls are vtable slots 3 (0xc/4) and 63 (0xfc/4) on the receiver and on the a1 pointer respectively; both go through an empty local class used purely for its member-pointer representation, which draws no C4234 and no extra symbol.
// size      289 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F4EC0
// indirect  0x005F4DC9 0x005F4DFD 0x005F4E10 0x005F4E3B 0x005F4EA3

extern "C" int __stdcall sub_5f4d90(void *self_ptr, void *a1) {
  class MfpBase {};
  typedef int (MfpBase::*Fn3)(int, int, int);
  typedef int (MfpBase::*Fn0)();
  union Conv3 {
    Fn3 asMfp;
    void *asPtr;
  };
  union Conv0 {
    Fn0 asMfp;
    void *asPtr;
  };

  char *self = (char *)self_ptr;
  if (a1 == 0) {
    return 0x10;
  }
  char *owner = *(char **)(self + 0xa8);
  if ((void *)owner == a1) {
    return 0;
  }
  char *a1b = (char *)a1;
  int left, top, right, bottom;
  if (owner == 0) {
    left = *(int *)(self + 0x13c);
    top = *(int *)(self + 0x140);
    right = *(int *)(self + 0x144);
    bottom = *(int *)(self + 0x148);
  } else {
    left = *(int *)(owner + 0x13c);
    top = *(int *)(owner + 0x140);
    right = *(int *)(owner + 0x144);
    bottom = *(int *)(owner + 0x148);
  }
  BOOL eq = left == *(int *)(a1b + 0x474) && top == *(int *)(a1b + 0x478) &&
            right == *(int *)(a1b + 0x47c) &&
            bottom == *(int *)(a1b + 0x480);
  if (!eq) {
    Conv3 c;
    c.asPtr = (*(void ***)a1)[0xc / 4];
    (reinterpret_cast<MfpBase *>(a1)->*c.asMfp)(right - left, bottom - top,
                                                 1);
  }
  *(void **)(self + 0xa8) = a1;

  int i = 0;
  WinZOrderCount = 0;
  if (*g_009b7b34 > 0) {
    void **arr = (void **)g_009b6e48;
    int cur = *g_009b7a6c;
    do {
      if (cur != 0 && cur == (int)arr[i]) {
        WinZOrderCount = 0;
        *g_009b7a78 = 0;
      }
      if (*(unsigned char *)((char *)arr[i] + 0x9c) & 1) {
        recurse_zorder((Win *)arr[i]);
        cur = *g_009b7a6c;
      }
      i = i + 1;
    } while (i < *g_009b7b34);
  }
  Conv0 c0;
  c0.asPtr = (*(void ***)self)[0xfc / 4];
  (reinterpret_cast<MfpBase *>(self)->*c0.asMfp)();
  return 0;
}
