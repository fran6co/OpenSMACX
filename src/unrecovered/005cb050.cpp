// ORIGINAL: 0x005CB050 sub_5cb050 0x005CB050-0x005CB220
// RULED-OUT: sub_5c9ec0 is catalogued nullary cdecl but the disassembly sets `ecx = esi` (this=a2) before every call - a free __thiscall declaration is C4234, so it is reached through the Dummy member-function-pointer trick. The four mmio* Windows multimedia calls go through fixed-address function-pointer globals rather than by name, since the catalogue does not model them. Landing the closest control-flow-faithful form (divergence starts at instruction #6, on the mmio-open guard).
// size      464 bytes
// prototype int (__thiscall sub_5CB050)(LPSTR pszFileName)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005C9EC0 0x00646ED0
// indirect  0x005CB06B 0x005CB096 0x005CB0B9 0x005CB0CC
// PROPOSAL: extern "C" int __fastcall sub_5cb050(LPSTR a1) ->
//           extern "C" int __fastcall sub_5cb050(LPSTR a1, int *a2) -
//           evidence: `mov esi, edx` at entry (the field/mmio object
//           lives in EDX, the fastcall convention's SECOND register), and
//           `push ecx` at entry stores `a1` into a stack local (`local_4`,
//           later used for the SEAD/SCHl/S1Nh magic-number probe) rather
//           than discarding it - matching Ghidra's own two-argument read.

typedef unsigned long DWORD;

typedef void *(__stdcall *MmioOpenAFn)(LPSTR, void *, DWORD);
typedef unsigned int(__stdcall *MmioSetBufferFn)(void *, LPSTR, long, unsigned int);
typedef long(__stdcall *MmioReadFn)(void *, char *, long);
typedef long(__stdcall *MmioSeekFn)(void *, long, int);

static int *const g_00669364 = (int *)0x00669364;
static int *const g_0066936c = (int *)0x0066936C;
static int *const g_00669370 = (int *)0x00669370;
static int *const g_00669374 = (int *)0x00669374;

class Dummy {
 public:
  int call0();
};
typedef int (Dummy::*MFP_Int0)();
union UInt0 {
  MFP_Int0 mfp;
  void *raw;
};

extern "C" int __fastcall sub_5cb050(LPSTR a1, int *a2) {
  a2[0xe6] = 0;
  LPSTR local4 = a1;

  if ((*reinterpret_cast<unsigned int *>(*a2 + 4) & 0x80000) == 0) {
    MmioOpenAFn mmioOpenA = reinterpret_cast<MmioOpenAFn>(*g_00669374);
    void *hmmio = mmioOpenA(a1, 0, 0);
    *reinterpret_cast<void **>(*a2 + 0xa0) = hmmio;
  }
  if (*reinterpret_cast<void **>(*a2 + 0xa0) == 0) {
    return 1;
  }

  MmioSetBufferFn mmioSetBuffer =
      reinterpret_cast<MmioSetBufferFn>(*g_00669370);
  unsigned int rc =
      mmioSetBuffer(*reinterpret_cast<void **>(*a2 + 0xa0), 0, 0x4000, 0);
  if (rc != 0) {
    return 1;
  }

  MmioReadFn mmioRead = reinterpret_cast<MmioReadFn>(*g_0066936c);
  mmioRead(*reinterpret_cast<void **>(*a2 + 0xa0),
           reinterpret_cast<char *>(&local4), 4);

  MmioSeekFn mmioSeek = reinterpret_cast<MmioSeekFn>(*g_00669364);
  mmioSeek(*reinterpret_cast<void **>(*a2 + 0xa0), -4, 1);

  int isKnown;
  if (local4 == reinterpret_cast<LPSTR>(0x44414553) ||
      local4 == reinterpret_cast<LPSTR>(0x6c484353) ||
      local4 == reinterpret_cast<LPSTR>(0x684e5331)) {
    isKnown = 1;
  } else {
    isKnown = 0;
  }
  *reinterpret_cast<int *>(*a2 + 0x7c) = isKnown;

  int *p = a2 + 0x19;
  for (int i1 = 0x18; i1 != 0; --i1) {
    *p = 0;
    p += 4;
  }
  p = a2 + 0x79;
  for (int i2 = 0x18; i2 != 0; --i2) {
    *p = 0;
    p += 4;
  }

  void *buf1 = calloc(1, 1200000);
  a2[7] = reinterpret_cast<int>(buf1);
  a2[9] = reinterpret_cast<int>(buf1);
  if (buf1 == 0) {
    return 2;
  }

  void *buf2 = calloc(1, 300000);
  a2[8] = reinterpret_cast<int>(buf2);
  a2[10] = reinterpret_cast<int>(buf2);
  if (buf2 == 0) {
    return 2;
  }

  int *p1 = reinterpret_cast<int *>(buf1);
  for (int i3 = 300000; i3 != 0; --i3) {
    *p1 = 0;
    ++p1;
  }
  int *p2 = reinterpret_cast<int *>(buf2);
  for (int i4 = 75000; i4 != 0; --i4) {
    *p2 = 0;
    ++p2;
  }

  a2[0xe2] = 0;
  a2[0xe3] = 0;
  a2[0xde] = 0;
  a2[0xdf] = 0;
  a2[0xe0] = 0;
  a2[0xe1] = 0;
  a2[0xe8] = 4;

  UInt0 u;
  u.raw = reinterpret_cast<void *>(&sub_5c9ec0);
  Dummy *self = reinterpret_cast<Dummy *>(a2);

  if (*reinterpret_cast<int *>(*a2 + 0x7c) == 0) {
    int v = a2[0xe0];
    while (v < 0x14 && (v = (self->*u.mfp)()) != -1) {
      v = a2[0xe0];
    }
  } else {
    int v = a2[0xe1];
    while (v < 0x15 && (v = (self->*u.mfp)()) != -1 &&
           (v = (self->*u.mfp)()) != -1) {
      v = a2[0xe1];
    }
  }

  *reinterpret_cast<unsigned int *>(*a2 + 0x80) = (a2[0xe2] == 0);
  return 0;
}
