// ORIGINAL: 0x005CAE80 sub_5cae80 0x005CAE80-0x005CB046
// RULED-OUT: 0x005CB050's scaffolded prototype takes only one fastcall param, but the real callee reads its object from EDX (`mov esi, edx` at its own entry) with ECX dead - called here through an explicit fastcall(LPSTR, int*) function-pointer cast rather than by name, since a second extern "C" declaration for the same symbol conflicts. The four thiscall callees (sub_5ce340/sub_5cbe30/sub_5cb220/sub_5ce450/ sub_5cc710) are similarly unreachable by name (VC6 C4234 on a free __thiscall) and go through the Dummy member-function- pointer trick. Landing the closest control-flow-faithful form (divergence starts at instruction #12, inside the calloc/zero preamble).
// size      454 bytes
// prototype int (__cdecl sub_5CAE80)(LPSTR pszFileName, int, int)
// callers   1   call targets   10
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005CB050 0x005CB220 0x005CBE30 0x005CC710 0x005CE340 0x005CE450 0x00644EF2 0x00646ED0 0x00646F5D 0x00647067
// indirect  0x005CAEF4 0x005CAF34 0x005CAF43 0x005CAF48 0x005CAF52 0x005CAFAD 0x005CB020 0x005CB027

extern "C" void *memset(void *, int, unsigned int);

typedef long(__stdcall *InterlockedExchangeFn)(long *, long);
typedef void(__stdcall *SleepFn)(unsigned int);
typedef unsigned int(__stdcall *MmioCloseFn)(void *, unsigned int);

static int *const g_00669100 = (int *)0x00669100;  // InterlockedExchange
static int *const g_0066912c = (int *)0x0066912C;  // Sleep
static int *const g_00669378 = (int *)0x00669378;  // mmioClose
static int *const g_005caab0 = (int *)0x005CAAB0;  // thread proc

class Dummy {
 public:
  void call0();
};
typedef void (Dummy::*MFP_Void)();
union UVoid {
  MFP_Void mfp;
  void *raw;
};

extern "C" int __cdecl sub_5cae80(LPSTR a1, int a2, int a3) {
  char *obj = reinterpret_cast<char *>(calloc(1, 0x660));
  if (obj == 0) {
    return 0;
  }

  memset(obj, 0, 0x660);
  *reinterpret_cast<int *>(obj) = a3;
  *reinterpret_cast<int *>(a3 + 0x9c) = a2;

  typedef int(__fastcall * Sub5cb050Fn)(LPSTR, int *);
  int rc = (reinterpret_cast<Sub5cb050Fn>(&sub_5cb050))(
      a1, reinterpret_cast<int *>(obj));
  if (rc != 0) {
    free(obj);
    return 0;
  }

  *reinterpret_cast<int *>(a3) = reinterpret_cast<int>(obj);

  UVoid u;
  u.raw = reinterpret_cast<void *>(&sub_5ce340);
  (reinterpret_cast<Dummy *>(obj)->*u.mfp)();

  u.raw = reinterpret_cast<void *>(&sub_5cbe30);
  (reinterpret_cast<Dummy *>(obj)->*u.mfp)();

  InterlockedExchangeFn interlocked =
      reinterpret_cast<InterlockedExchangeFn>(*g_00669100);
  interlocked(reinterpret_cast<long *>(a3 + 0x98), 0);

  if (a3 == 0) {
    return 0;
  }

  *reinterpret_cast<int *>(obj + 0x3d0) = 1;
  int flags = *reinterpret_cast<int *>(a3 + 4);
  if ((flags & 0x10000) != 0) {
    int threadHandle =
        _beginthread(reinterpret_cast<int>(g_005caab0), 0, a3);
    *reinterpret_cast<int *>(a3 + 0xa8) = threadHandle;
    if (threadHandle == -1) {
      return 0;
    }

    long *flagAddr = reinterpret_cast<long *>(a3 + 0x98);
    if (interlocked(flagAddr, 1) != 2) {
      SleepFn sleepFn = reinterpret_cast<SleepFn>(*g_0066912c);
      do {
        sleepFn(0x14);
      } while (interlocked(flagAddr, 1) != 2);
    }
    interlocked(flagAddr, 2);
    return a3;
  }

  u.raw = reinterpret_cast<void *>(&sub_5cb220);
  (reinterpret_cast<Dummy *>(a3)->*u.mfp)();

  bool endThreadNext = false;
  if (*reinterpret_cast<int *>(obj + 0x3d0) != 0) {
    int stride = *reinterpret_cast<int *>(obj + 0x60c) -
                 *reinterpret_cast<int *>(obj + 0x608);
    *reinterpret_cast<int *>(a3 + 0x8c) = stride;

    if ((*reinterpret_cast<int *>(a3 + 4) & 0x80000) == 0) {
      int hmmio = *reinterpret_cast<int *>(a3 + 0xa0);
      if (hmmio != 0) {
        MmioCloseFn mmioClose =
            reinterpret_cast<MmioCloseFn>(*g_00669378);
        mmioClose(reinterpret_cast<void *>(hmmio), 0);
      }
    }

    if (*reinterpret_cast<int *>(obj + 0x1c) != 0) {
      free(*reinterpret_cast<void **>(obj + 0x1c));
    }
    if (*reinterpret_cast<int *>(obj + 0x20) != 0) {
      free(*reinterpret_cast<void **>(obj + 0x20));
    }

    char *p = obj + 0x1e4;
    for (int i = 0x18; i != 0; --i) {
      *reinterpret_cast<int *>(p) = 0;
      p += 0x10;
    }

    if (*reinterpret_cast<int *>(a3 + 0x7c) != 0) {
      char *p2 = obj + 0x64;
      for (int i = 0x18; i != 0; --i) {
        *reinterpret_cast<int *>(p2) = 0;
        p2 += 0x10;
      }
    }

    u.raw = reinterpret_cast<void *>(&sub_5ce450);
    (reinterpret_cast<Dummy *>(obj)->*u.mfp)();

    u.raw = reinterpret_cast<void *>(&sub_5cc710);
    (reinterpret_cast<Dummy *>(obj)->*u.mfp)();

    interlocked(reinterpret_cast<long *>(a3 + 0x98), 6);
    interlocked(reinterpret_cast<long *>(a3), 0);
    free(obj);
  }

  if (!endThreadNext) {
    return 0;
  }
  _endthread();
  return 0;
}
