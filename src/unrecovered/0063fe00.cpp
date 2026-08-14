// ORIGINAL: 0x0063FE00
// name      sub_63fe00
// size      477 bytes
// spans     0x0063FE00-0x0063FFDD
// prototype
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006281B0 0x00628220 0x006458C8
// PROPOSAL: extern "C" int __cdecl sub_63fe00() ->
//           extern "C" void __cdecl sub_63fe00(unsigned char *param_1) -
//           evidence: `mov eax, [esp+4]` (single stack arg before any
//           push) then `mov cl, byte ptr [eax]` - a pointer, not a plain
//           int, matching IDA's one-argument guess; no explicit `mov eax,`
//           before `ret` (see the sibling 0x0063FFE0's identical tail).
// RULED-OUT: near-duplicate of 0x0063FFE0 past the preamble (same
//            628220/6281b0/round-to-nearest/octant/ratio tail) - only the
//            first block differs, negating all three floats when
//            `*param_1 & 1` instead of calling sub_627f50. The three
//            round-to-nearest computations are inlined at each call site
//            rather than factored into a shared helper: a free-function
//            helper ahead of the definition compiles to zero external
//            .text symbols in body-mode extraction (only a helper CLASS is
//            special-cased). Landing the closest control-flow-faithful
//            form (divergence starts at instruction #2, on the flag test).

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
static int *const g_009c0ac8 = (int *)0x009C0AC8;
static int *const g_009c0b74 = (int *)0x009C0B74;

extern "C" void __cdecl sub_63fe00(unsigned char *param_1) {
  float local18;
  float local14;
  float local10;

  if ((*param_1 & 1) != 0) {
    local18 = -*g_009c0aa8;
    local14 = -*g_009c0ab4;
    local10 = -*g_009c0ac0;
  } else {
    local18 = *g_009c0aa8;
    local14 = *g_009c0ab4;
    local10 = *g_009c0ac0;
  }

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
