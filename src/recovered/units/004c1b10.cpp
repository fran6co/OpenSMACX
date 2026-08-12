// ORIGINAL: 0x004C1B10 FILE
// name      ?vehdraw_compute_facet_normals@@YAXXZ
// size      487 bytes
// spans     0x004C1B10-0x004C1CF7
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00628220 0x00628290
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004C1B10
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c1b10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?vehdraw_compute_facet_normals@@YAXXZ  at 0x004C1B10  (487 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl sub_628220();
extern "C" int __cdecl sub_628290();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_008d4d9c = (int *)0x008D4D9C;
static int *const g_008dbe28 = (int *)0x008DBE28;
static int *const g_0093341c = (int *)0x0093341C;

// sub_628220/sub_628290 are called with arguments (arity from
// prototype-hypotheses.csv: 2 and 3 respectively), but the emitter's slot
// for both is nullary since neither has a mangled name to decode. Extra
// shims with the real signatures stand in; the call target's relocation is
// masked by the comparison so the shim's own name does not matter.
struct Vec3 { float x, y, z; };
extern "C" void __cdecl sub_628290_shim(Vec3 *a, Vec3 *b, Vec3 *out);
extern "C" void __cdecl sub_628220_shim(Vec3 *out, Vec3 *in);

// esi walks this table forward by one Vec3 (0xc bytes) per innermost
// iteration and is also used to compute -0x708C/+0x708C/+0xE118 offsets
// into neighboring tables - the address itself does arithmetic, so it
// needs `extern`, not the plain const-pointer spelling above.
extern int g_008d4d9c_tbl[];

void __cdecl vehdraw_compute_facet_normals() {
    int scale = *g_0093341c;
    Vec3 *pos = (Vec3 *)g_008d4d9c_tbl;

    for (int i = 0x1e; i < 0xf0; i += 0x1e) {
        float fi = (float)((i >> 2) - scale);
        for (int j = 0x1e; j < 0xf0; j += 0x1e) {
            float fj = (float)((j >> 2) - scale);
            for (int k = 0x1e; k < 0xf0; k += 0x1e) {
                float fk = (float)((k >> 2) - scale);
                for (int l = 0x1e; l < 0xf0; l += 0x1e) {
                    float fl = (float)((l >> 2) - scale);

                    Vec3 a = { 0.0f, 50.0f, fj };
                    Vec3 b = { -50.0f, 0.0f, fi };
                    Vec3 c = { 50.0f, 0.0f, fk };
                    Vec3 d = { 0.0f, -50.0f, fl };

                    Vec3 n0, n1, n2, n3;
                    sub_628290_shim(&a, &b, &n0);
                    sub_628290_shim(&c, &a, &n1);
                    sub_628290_shim(&d, &c, &n2);
                    sub_628290_shim(&b, &d, &n3);

                    sub_628220_shim(&n0, &n0);
                    sub_628220_shim(&n1, &n1);
                    sub_628220_shim(&n2, &n2);
                    sub_628220_shim(&n3, &n3);

                    *(Vec3 *)((char *)pos - 0x708C) = n0;
                    *pos = n1;
                    *(Vec3 *)((char *)pos + 0x708C) = n2;
                    *(Vec3 *)((char *)pos + 0xE118) = n3;
                    pos++;
                }
            }
        }
    }
}
