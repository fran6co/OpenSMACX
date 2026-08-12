// ORIGINAL: 0x005C95E0 FILE
// name      ?adjust_pal@Gamma@@QAEXN@Z
// size      465 bytes
// spans     0x005C95E0-0x005C97B1
// prototype void (__thiscall ?adjust_pal@Gamma@@QAEXN@Z)(Gamma* this, unknown float)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FE460 0x005FE6D0 0x006458C8 0x00646A99
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005C95E0
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005c95e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?adjust_pal@Gamma@@QAEXN@Z  at 0x005C95E0  (465 bytes)
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

class Gamma;
class Palette;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Palette { public:
    int set();
    void reseed();
};
extern "C" int __cdecl _ftol();
extern "C" int __cdecl pow();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066d8d8 = (int *)0x0066D8D8;
static int *const g_0066fa68 = (int *)0x0066FA68;
static int *const g_0066fa70 = (int *)0x0066FA70;
static int *const g_0066fa78 = (int *)0x0066FA78;

class Gamma { public:
    void adjust_pal(double);
};

typedef double(__cdecl *PowFn)(double, double);
#define POW (reinterpret_cast<PowFn>(&pow))

void Gamma::adjust_pal(double a1) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0xa64) != 0) {
        double invGamma = *reinterpret_cast<double *>(g_0066d8d8) / a1;
        int i = 0;
        do {
            unsigned char *pal = *reinterpret_cast<unsigned char **>(self + 0xa64);
            double rN = static_cast<double>(pal[i + 0x28]) * *reinterpret_cast<double *>(g_0066fa78);
            double gN = static_cast<double>(pal[i + 0x29]) * *reinterpret_cast<double *>(g_0066fa78);
            double bN = static_cast<double>(pal[i + 0x2a]) * *reinterpret_cast<double *>(g_0066fa78);

            double r = POW(rN, invGamma);
            double g = POW(gN, invGamma);
            double b = POW(bN, invGamma);

            if (r < *reinterpret_cast<double *>(g_0066fa70)) {
                r = *reinterpret_cast<double *>(g_0066fa70);
            } else if (r > *reinterpret_cast<double *>(g_0066d8d8)) {
                r = *reinterpret_cast<double *>(g_0066d8d8);
            }
            if (g < *reinterpret_cast<double *>(g_0066fa70)) {
                g = *reinterpret_cast<double *>(g_0066fa70);
            } else if (g > *reinterpret_cast<double *>(g_0066d8d8)) {
                g = *reinterpret_cast<double *>(g_0066d8d8);
            }
            if (b < *reinterpret_cast<double *>(g_0066fa70)) {
                b = *reinterpret_cast<double *>(g_0066fa70);
            } else if (b > *reinterpret_cast<double *>(g_0066d8d8)) {
                b = *reinterpret_cast<double *>(g_0066d8d8);
            }

            unsigned char rByte = static_cast<unsigned char>(static_cast<int>(r * *reinterpret_cast<double *>(g_0066fa68)));
            unsigned char gByte = static_cast<unsigned char>(static_cast<int>(g * *reinterpret_cast<double *>(g_0066fa68)));
            unsigned char bByte = static_cast<unsigned char>(static_cast<int>(b * *reinterpret_cast<double *>(g_0066fa68)));

            unsigned char *pal2 = *reinterpret_cast<unsigned char **>(self + 0xa64);
            pal2[i + 0x28] = rByte;
            pal2[i + 0x29] = gByte;
            pal2[i + 0x2a] = bByte;

            i += 4;
        } while (i < 0x3b0);

        reinterpret_cast<Palette *>(*reinterpret_cast<int *>(self + 0xa64))->reseed();
        reinterpret_cast<Palette *>(*reinterpret_cast<int *>(self + 0xa64))->set();
    }
}
