// ORIGINAL: 0x0063FFE0 sub_63ffe0 0x0063FFE0-0x006401A8
// TRIED: the three callees (sub_627f50/sub_628220/sub_6281b0) are catalogued nullary, so they are reached through explicit cdecl function-pointer casts rather than by name. The first round-to-nearest computation tests the sign of localC[0] (the sub_628220 output) but rounds local18 (the sub_6281b0 output) - an asymmetry the disassembly shows plainly and this keeps, even though axes 1 and 2 are self-referential. A shared `round_nearest` free-function helper ahead of the definition compiled to zero external .text symbols (body-mode extraction only special-cases a helper CLASS, not a helper free function) - inlined the three round computations instead. Landing the closest control-flow-faithful form (divergence starts at instruction #12, inside the float-copy preamble).
// size      456 bytes
// prototype
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00627F50 0x006281B0 0x00628220 0x006458C8
// PROPOSAL: extern "C" int __cdecl sub_63ffe0() ->
//           extern "C" void __cdecl sub_63ffe0(int a1) - evidence: the
//           single stack read at [esp+0x24] before any push matches IDA's
//           and Ghidra's one-argument guess, and there is no explicit `mov
//           eax,` before `ret` - the value in eax at return is the `shl`
//           result incidentally, not a deliberate return.

static float *const g_00671038 = (float *)0x00671038;
static float *const g_0067103c = (float *)0x0067103C;
static float *const g_00671040 = (float *)0x00671040;
static float *const g_00671044 = (float *)0x00671044;
static float *const g_009c0aa8 = (float *)0x009C0AA8;
static float *const g_009c0ab4 = (float *)0x009C0AB4;
static float *const g_009c0ac0 = (float *)0x009C0AC0;
static int *const g_009c0a70 = (int *)0x009C0A70;
static int *const g_009c0a74 = (int *)0x009C0A74;
static int *const g_009c0a78 = (int *)0x009C0A78;
static int *const g_009c0ac4 = (int *)0x009C0AC4;
static int *const g_009c0ac8 = (int *)0x009C0AC8;
static int *const g_009c0b74 = (int *)0x009C0B74;
static void *const g_009c0aa0 = (void *)0x009C0AA0;

extern "C" void __cdecl sub_63ffe0(int a1) {
  typedef void(__cdecl * Sub627f50Fn)(int, void *);
  (reinterpret_cast<Sub627f50Fn>(&sub_627f50))(a1, g_009c0aa0);

  float local18 = *g_009c0aa8;
  float local14 = *g_009c0ab4;
  *g_009c0ac4 = 1;
  float local10 = *g_009c0ac0;

  float localC[3];
  typedef void(__cdecl * Sub628220Fn)(float *, float *);
  (reinterpret_cast<Sub628220Fn>(&sub_628220))(&local18, localC);

  typedef void(__cdecl * Sub6281b0Fn)(float *, float, float *);
  (reinterpret_cast<Sub6281b0Fn>(&sub_6281b0))(localC, 256.0f, &local18);

  float addX;
  if (localC[0] < *g_0067103c) {
    addX = *g_00671040;
  } else {
    addX = *g_00671044;
  }
  int ix = (int)(local18 + addX);
  *g_009c0a70 = ix;

  float addY;
  if (local14 < *g_0067103c) {
    addY = *g_00671040;
  } else {
    addY = *g_00671044;
  }
  int iy = (int)(local14 + addY);
  *g_009c0a74 = iy;

  float addZ;
  if (local10 < *g_0067103c) {
    addZ = *g_00671040;
  } else {
    addZ = *g_00671044;
  }
  int iz = (int)(local10 + addZ);
  *g_009c0a78 = iz;

  char octant;
  if (iy < 1) {
    octant = (char)((ix > 0) + 2);
  } else {
    octant = (char)(ix < 1);
  }
  if (iz < 0) {
    octant = (char)(octant + 4);
  }

  int absIx = ix < 0 ? -ix : ix;
  int absIy = iy < 0 ? -iy : iy;
  int absIz = iz < 0 ? -iz : iz;
  int sum = absIx + absIy + absIz;

  float ratio = *g_00671038 / (float)sum;
  int fx = (int)(absIx * ratio);
  int fy = (int)(absIy * ratio);

  unsigned char bits;
  if (fx > 0x80) {
    bits = 2;
  } else if (fy > 0x80) {
    bits = 3;
  } else {
    bits = (unsigned char)(fy <= 0x80 - fx);
  }

  *g_009c0ac8 = 1;
  *g_009c0b74 = 1 << (octant * 4 | bits);
}
