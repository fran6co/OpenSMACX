// ORIGINAL: 0x00628B70 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00628B70
// name           ?HSV_to_RGB@@YAXPAUHSV@@PAUPALETTEENTRY@@@Z
// size           546 bytes
// measured tier  MISMATCH
// divergence     146
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00628b70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?HSV_to_RGB@@YAXPAUHSV@@PAUPALETTEENTRY@@@Z  at 0x00628B70  (546 bytes)
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

struct HSV;
struct PALETTEENTRY;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl _ftol();
extern "C" int __cdecl floor();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00628d94 = (int *)0x00628D94;
static int *const g_0066d8d8 = (int *)0x0066D8D8;
static int *const g_0066fa70 = (int *)0x0066FA70;
static int *const g_00670c20 = (int *)0x00670C20;
static int *const g_00670c28 = (int *)0x00670C28;
static int *const g_00670c30 = (int *)0x00670C30;
static int *const g_00670c38 = (int *)0x00670C38;

// Completes the opaque forward declarations with the layout the rep movsd
// (6 dwords = 3 doubles) and the three-byte RGB stores require.
struct HSV { double h; double s; double v; };
struct PALETTEENTRY { unsigned char r, g, b, flags; };

// `floor` is catalogued nullary (see _ftol above), but this call site pushes
// a real double argument before it - shim with the actual convention; the
// callee's spelling never reaches the comparison, only convention/params/
// return type do.
extern "C" double __cdecl c_floor(double);

void __cdecl HSV_to_RGB(HSV * a1, PALETTEENTRY * a2) {
    if (a1 != 0 && a2 != 0) {
        HSV hsv = *a1;
        double r, g, b;

        if (hsv.s == 0.0) {
            if (hsv.h == -1.0) {
                r = hsv.v;
                g = hsv.v;
                b = hsv.v;
            }
        } else {
            if (hsv.h == 360.0)
                hsv.h = 0.0;
            hsv.h *= 0.016666666666666666;
            int i = (int)c_floor(hsv.h);
            double f = hsv.h - i;
            double p = hsv.v * (1.0 - hsv.s);
            double q = hsv.v * (1.0 - hsv.s * f);
            double t = hsv.v * (1.0 - hsv.s * (1.0 - f));
            switch (i) {
            case 0: r = hsv.v; g = t; b = p; break;
            case 1: r = q; g = hsv.v; b = p; break;
            case 2: r = p; g = hsv.v; b = t; break;
            case 3: r = p; g = q; b = hsv.v; break;
            case 4: r = t; g = p; b = hsv.v; break;
            case 5: r = hsv.v; g = p; b = q; break;
            }
        }

        a2->r = (unsigned char)(r * 256.0);
        a2->g = (unsigned char)(g * 256.0);
        a2->b = (unsigned char)(b * 256.0);
    }
}
