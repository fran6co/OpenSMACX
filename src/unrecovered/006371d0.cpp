// ORIGINAL: 0x006371D0 Video_unk10 0x006371D0-0x006372F7
// RULED-OUT: reads at 0x4b0/0x4ac/0x100c/0x4c0/0x1000/0x1004 with the receiver cached to esi and no matching stack slot mean __thiscall on a Video object, but Video_unk14 (0x638090) and sub_63e540 (0x63e540) are BOTH called with the SAME receiver in ecx too - a second thiscall convention the scaffold's own nullary/stdcall fallback declarations for those two callees cannot express without conflicting with the declaration already in this translation unit. Landed the calls to those two exactly as the scaffold declared them (stdcall, no receiver), which compiles and keeps the call target addresses right but drops the ecx setup at those two call sites - a real, local divergence rather than a byte-exact ABI. The decode_frame/read_frame calls reuse Video's own catalogued declarations directly. mmioSeek/mmioDescend are read through their import-table globals with locally-declared signatures, since the scaffold does not carry MMIO types.
// size      295 bytes
// prototype
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00637910 0x00637C30 0x00638090 0x0063E540
// indirect  0x0063722F 0x0063725C

extern "C" int __stdcall Video_unk10(Video *self, int a1) {
  char *e = (char *)self;

  if (*(int *)(e + 0x4b0) == 0) {
    return 7;
  }
  if ((*(unsigned char *)(e + 0x4ac) & 1) == 0) {
    return 0;
  }

  int frame = Video_unk14(a1);
  if (frame < 0) {
    return 1;
  }

  typedef long LONG;
  typedef void *HMMIO;
  typedef unsigned int MMRESULT;
  typedef LONG(__stdcall * MmioSeekFn)(HMMIO, LONG, int);
  MmioSeekFn mmioSeekFn = (MmioSeekFn) * g_00669364;
  int *field100c = *(int **)(e + 0x100c);
  LONG pos = mmioSeekFn(*(HMMIO *)(e + 0x4b0),
                        *(int *)((char *)field100c + 8) + *(int *)(e + 0x4c0),
                        0);
  if (pos == -1) {
    return 1;
  }

  typedef MMRESULT(__stdcall * MmioDescendFn)(HMMIO, void *, void *, UINT);
  MmioDescendFn mmioDescendFn = (MmioDescendFn) * g_0066938c;
  MMRESULT mr =
      mmioDescendFn(*(HMMIO *)(e + 0x4b0), e + 0x4c8, e + 0x4b4, 0x10);
  if (mr != 0) {
    return 1;
  }

  sub_63e540(0x28);

  *(int *)(e + 0x1000) = frame;
  *(int *)(e + 0x1004) = 0;

  if (0 < a1 - 1) {
    do {
      *(unsigned int *)(e + 0x4ac) &= 0xffffefff;
      int r = self->decode_frame(1);
      if (r == 0) {
        self->read_frame();
      }
    } while (*(int *)(e + 0x1004) < a1 - 1);
  }

  *(unsigned int *)(e + 0x4ac) &= 0xffffefff;
  {
    int r = self->decode_frame(0);
    if (r == 0) {
      self->read_frame();
      self->decode_frame(0);
    }
  }
  return 0;
}
