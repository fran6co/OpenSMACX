// ORIGINAL: 0x0045F950
// name      _WinMain@16
// size      459 bytes
// spans     0x0045F950-0x0045FB1B
// prototype int (__stdcall _WinMain@16)(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
// callers   1   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C5CE0 0x0052AA30 0x0059DB40 0x0059E530 0x006185A0 0x00618D20 0x006190D0 0x0062D3A0 0x0062D500 0x0062D570 0x00645470
// indirect  0x0045F9DE 0x0045FAC9
// RULED-OUT: Caviar::init_class/close_class are declared non-static in
//            this scaffold (unlike Font's, which the emitter marks
//            static), so `Caviar::init_class()` fails C2352; the
//            disassembly never sets up ecx before either call, so they are
//            reached through a null `Caviar*` receiver instead. `a3`
//            (lpCmdLine) is reused as a plain int scratch after the
//            strcat, matching the original's stack-slot reuse of the same
//            parameter. Landing the closest control-flow-faithful form
//            (divergence starts at instruction #1, on the prologue shape).

typedef void(__cdecl *HandlerFn)();
typedef int(__stdcall *GetSystemMetricsFn)(int);
typedef short(__stdcall *GetAsyncKeyStateFn)(int);

static int *const g_00404fb0 = (int *)0x00404FB0;
static int *const g_00604e40 = (int *)0x00604E40;
static int *const g_00669330 = (int *)0x00669330;
static int *const g_00669334 = (int *)0x00669334;
static int *const g_006853c0 = (int *)0x006853C0;
static int *const g_006853cc = (int *)0x006853CC;
static int *const g_006853d8 = (int *)0x006853D8;
static int *const g_006853f4 = (int *)0x006853F4;
static int *const g_006853fc = (int *)0x006853FC;
static int *const g_00685404 = (int *)0x00685404;
static int *const g_00685408 = (int *)0x00685408;
static int *const g_0068540c = (int *)0x0068540C;
static int *const g_00685418 = (int *)0x00685418;
static int *const g_00691b2c = (int *)0x00691B2C;
static int *const g_00696ecc = (int *)0x00696ECC;
static int *const g_006970dc = (int *)0x006970DC;
static int *const g_007d3948 = (int *)0x007D3948;
static int *const g_007d3970 = (int *)0x007D3970;
static int *const g_009469fc = (int *)0x009469FC;
static int *const g_0094c590 = (int *)0x0094C590;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009b7b28 = (int *)0x009B7B28;

extern "C" int __stdcall WinMain(HINSTANCE a1, HINSTANCE a2, LPSTR a3, int a4) {
  *reinterpret_cast<char *>(g_007d3970) = 0;
  if (a3 != 0) {
    strcat(reinterpret_cast<char *>(g_007d3970), a3);
  }

  HandlerFn handler = reinterpret_cast<HandlerFn>(g_00404fb0);
  *g_009a6488 = 1;
  *g_009469fc = 0;
  *reinterpret_cast<HandlerFn *>(g_00696ecc) =
      reinterpret_cast<HandlerFn>(g_00604e40);
  if (handler) {
    *reinterpret_cast<HandlerFn *>(g_00696ecc) = handler;
  }

  *g_006970dc = *g_006970dc | 4;
  int videoMode = prefs_get(reinterpret_cast<char *>(g_006853c0), 0, 0);
  int useDirectDraw = prefs_get(reinterpret_cast<char *>(g_006853cc), 1, 0);

  int width;
  int height;
  int bpp;
  if (useDirectDraw != 0) {
    if (videoMode == 0) {
      GetSystemMetricsFn getSystemMetrics =
          reinterpret_cast<GetSystemMetricsFn>(*g_00669334);
      videoMode = getSystemMetrics(0);
    }
    if (videoMode < 0x400) {
      bpp = 8;
      height = 600;
      width = 800;
    } else {
      bpp = 8;
      height = 0x300;
      width = 0x400;
    }
    a3 = reinterpret_cast<LPSTR>(8);
  } else {
    bpp = reinterpret_cast<int>(a3);
    width = reinterpret_cast<int>(a3);
    height = reinterpret_cast<int>(a3);
  }

  int versionOk = jackal_version_check(reinterpret_cast<char *>(g_00685418));
  if (versionOk != 0) {
    return 0;
  }

  int ddFlag = -(useDirectDraw != 0) & 4;
  int initOk = jackal_init_real(
      reinterpret_cast<Palette *>(g_0094c590),
      reinterpret_cast<Font *>(g_007d3948),
      reinterpret_cast<char *>(g_006853d8), ddFlag, width, height, bpp);
  if (initOk == 0) {
    return 0;
  }

  if (reinterpret_cast<Caviar *>(0)->init_class() != 0) {
    return 0;
  }

  int soundArg = 10;
  if (prefs_get(reinterpret_cast<char *>(g_006853f4), 1, 0) != 0) {
    soundArg = 0x2a;
    prefs_put(reinterpret_cast<char *>(g_006853fc), 1, 0);
    if (prefs_get(reinterpret_cast<char *>(g_00685404), 1, 0) != 0) {
      soundArg = 0x6a;
      prefs_put(reinterpret_cast<char *>(g_00685408), 1, 0);
    }
  }

  GetAsyncKeyStateFn getAsyncKeyState =
      reinterpret_cast<GetAsyncKeyStateFn>(*g_00669330);
  short keyState = getAsyncKeyState(0x10);
  if ((char)((unsigned short)keyState >> 8) == 0) {
    init_sound(*reinterpret_cast<void **>(g_009b7b28), soundArg);
  }

  reinterpret_cast<Font *>(g_007d3948)
      ->init(reinterpret_cast<char *>(g_0068540c),
             *reinterpret_cast<char **>(g_00691b2c), 0xc, 0);
  control_game();
  reinterpret_cast<Caviar *>(0)->close_class();
  jackal_close();
  return 1;
}
