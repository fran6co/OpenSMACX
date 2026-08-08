// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00628DB0
// name           ?RGB_to_HSV@@YAXPAUPALETTEENTRY@@PAUHSV@@@Z
// size           375 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00628db0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?RGB_to_HSV@@YAXPAUPALETTEENTRY@@PAUHSV@@@Z  at 0x00628DB0  (375 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066fa70 = (int *)0x0066FA70;
static int *const g_0066fdc8 = (int *)0x0066FDC8;
static int *const g_00670c30 = (int *)0x00670C30;
static int *const g_00670c40 = (int *)0x00670C40;
static int *const g_00670c48 = (int *)0x00670C48;
static int *const g_00670c50 = (int *)0x00670C50;

// MISMATCH (#2: original 'and esp,0xfffffff8' vs rebuilt 'sub' - the
// original 8-byte-aligns and reserves 0x18 bytes of STACK-RESIDENT scratch
// (three qword slots it stores intermediate FP results into with
// `fst`/`fstp` mid-expression, per 0x628DFD/0x628E0B/0x628EA4), which this
// straight `if`-ladder translation of the max/min never needs - every local
// here lives purely in the x87 stack, so the compiler never reserves or
// aligns a frame for it. The FPU compare/conditional-load shape in the
// original (fcom; fnstsw ax; test ah,N; jne; fld st(n)) is also denser than
// a plain `if (a<b) x=b;` chain would produce - it reads like a min/max
// expressed through nested ternaries or a min3()/max3() helper the
// catalogue does not name, not the simple ladder Ghidra hypothesized. Tried
// the literal Ghidra-shaped translation once, plus hoisting the byte->int
// RGB conversion into named locals to see if that alone forced the
// alignment prologue - it did not. Not chased further: this is the
// documented INSTRUCTION SCHEDULING/x87-stack class of wall, and getting a
// spelling that reproduces the exact FPU stack traffic would need
// experimentally guessing the original expression tree, not a source-form
// fix visible from here.
void __cdecl RGB_to_HSV(PALETTEENTRY * a1, HSV * a2) {
    unsigned char *p = reinterpret_cast<unsigned char *>(a1);
    double *out = reinterpret_cast<double *>(a2);
    if (a2 != 0 && a1 != 0) {
        const double scale = *reinterpret_cast<const double *>(g_00670c50);
        const double zero = *reinterpret_cast<const double *>(g_0066fa70);
        int ir = p[0];
        int ig = p[1];
        int ib = p[2];
        double r = static_cast<double>(ir) * scale;
        double g = static_cast<double>(ig) * scale;
        double b = static_cast<double>(ib) * scale;
        double vmax = r;
        if (r < g) vmax = g;
        if (vmax < b) vmax = b;
        double vmin = r;
        if (g < r) vmin = g;
        if (b < vmax) vmin = b;
        out[2] = vmax;
        double s = zero;
        if (vmax != zero) s = (vmax - vmin) / vmax;
        out[1] = s;
        if (s == zero) {
            *reinterpret_cast<int *>(out) = 0;
            *(reinterpret_cast<int *>(out) + 1) = static_cast<int>(0xbff00000);
            return;
        }
        double delta = vmax - vmin;
        double h;
        if (r == vmax) {
            h = (g - b) / delta;
        } else if (g == vmax) {
            h = (b - r) / delta + *reinterpret_cast<const double *>(g_00670c48);
        } else {
            h = (r - g) / delta + *reinterpret_cast<const double *>(g_0066fdc8);
        }
        out[0] = h;
        double h2 = out[0] * *reinterpret_cast<const double *>(g_00670c40);
        bool neg = h2 < zero;
        out[0] = h2;
        if (neg) {
            out[0] = h2 + *reinterpret_cast<const double *>(g_00670c30);
        }
    }
}
