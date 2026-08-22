// ORIGINAL: 0x005A94F0 ?save_daemon@@YAHPAD@Z 0x005A94F0-0x005A96C3
// TRIED: `fclose`/`remove` are catalogued nullary cdecl, so they are reached through explicit function-pointer casts rather than by name (a direct call fails C2660: too many arguments). The return-value accumulator (`edi`) starts at 2 before `header_write`, becomes 3 once the first `fwrite` succeeds, and only reaches 0 once `game_data` and the second `fwrite` both succeed - the fclose/remove tail runs unconditionally, and `remove` only when the status is still nonzero. Landing the closest control-flow-faithful form (divergence starts at instruction #6, on the `inc` vs. explicit reload of the reentrancy counter).
// size      467 bytes
// prototype
// callers   6   call targets   10
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00538FB0 0x0057D240 0x005A6AC0 0x00634BB0 0x00645470 0x00645598 0x00645930 0x00645DD0 0x0064603F 0x00646950
// indirect  0x005A950A

typedef unsigned long DWORD;
typedef DWORD(__stdcall *GetTickCountFn)();

static int *const g_00669368 = (int *)0x00669368;
static int *const g_00690d80 = (int *)0x00690D80;
static int *const g_00690d84 = (int *)0x00690D84;
static int *const g_00690d88 = (int *)0x00690D88;
static int *const g_00691b34 = (int *)0x00691B34;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093a9b4 = (int *)0x0093A9B4;
static void *const g_0093d4f8 = (void *)0x0093D4F8;
static void *const g_0093d4fc = (void *)0x0093D4FC;
static void *const g_0093e978 = (void *)0x0093E978;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f798 = (int *)0x0093F798;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a6810 = (int *)0x009A6810;
static int *const g_009a681c = (int *)0x009A681C;

int __cdecl save_daemon(char *a1) {
  if (*g_0093a95c != 0) {
    ++*g_0093a9b4;
  }

  GetTickCountFn getTickCount = reinterpret_cast<GetTickCountFn>(*g_00669368);
  DWORD tick = getTickCount();
  int local4 = tick;

  if ((*g_009a649c & 0x400) != 0 && *g_0093f660 == 0) {
    my_srand(tick);
  }

  *g_009a6810 = 0xc;
  int flagBit = 0x10000000;
  if ((*g_009a649c & 0x400) != 0) {
    *g_0093f798 = 0;
    *g_009a64c0 = *g_009a64c0 | flagBit;
  }

  if (*g_0093a95c != 0) {
    *g_009a681c = *g_009a681c | 0x20;
    *g_009a64c0 = *g_009a64c0 | flagBit;
    *g_0093f798 = 0;
  }

  if (*g_0093f660 != 0) {
    *g_009a64c0 = *g_009a64c0 | flagBit;
    *g_0093f798 = *reinterpret_cast<int *>(g_0093d4f8);
    memcpy(g_0093e978, g_0093d4fc, 0xce0);
  }

  char localBuf[0x100];
  int local8;

  localBuf[0] = 0;
  strcat(localBuf, a1);
  if (strchr(localBuf, '.') == 0) {
    strcat(localBuf, reinterpret_cast<char *>(g_00690d80));
    strcat(localBuf, *reinterpret_cast<char **>(g_00691b34));
  }

  FILE *fp = reinterpret_cast<FILE *>(
      env_open(localBuf, reinterpret_cast<char *>(g_00690d84)));
  if (fp == 0) {
    return 1;
  }

  header_write(reinterpret_cast<char *>(g_00690d88), fp);

  int status = 2;
  local8 = 0x56;
  if (_fwrite(&local8, 4, 1, fp) != 0) {
    status = 3;
    if (game_data(fp, 1) == 0) {
      if (_fwrite(&local4, 4, 1, fp) != 0) {
        status = 0;
      }
    }
  }

  typedef int(__cdecl * FcloseFn)(FILE *);
  (reinterpret_cast<FcloseFn>(&fclose))(fp);
  if (status != 0) {
    typedef int(__cdecl * RemoveFn)(const char *);
    (reinterpret_cast<RemoveFn>(&remove))(localBuf);
  }
  return status;
}
