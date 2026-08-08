// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00439C00
// name           ?check_abil@DesignWin@@QAEXXZ
// size           292 bytes
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439c00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?check_abil@DesignWin@@QAEXXZ  at 0x00439C00  (292 bytes)
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

class DesignWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
bool __cdecl has_tech(int, int);
int __cdecl abil_cond(int, int, int, int);
int __cdecl bit_count(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00949780 = (int *)0x00949780;

class DesignWin { public:
    void check_abil();
};

// INDEXED TABLE BASE: the emitter did not pick this global up because it is
// used as `[eax + 0x94a37d]` with a computed index, not a plain load/store.
extern uint8_t g_0094a37d[];

void DesignWin::check_abil() {
    char *self = reinterpret_cast<char *>(this);
#define F141FC (*reinterpret_cast<int *>(self + 0x141FC))
#define F14200 (*reinterpret_cast<int *>(self + 0x14200))
#define F14204 (*reinterpret_cast<int *>(self + 0x14204))
#define F14208 (*reinterpret_cast<unsigned int *>(self + 0x14208))
#define F14218(k) (*reinterpret_cast<int *>(self + 0x14218 + (k) * 4))
#define F141F4 (*reinterpret_cast<int *>(self + 0x141F4))

    if (g_0094a37d[F141FC * 144] != 0) {
        F14208 = 0;
        int *slots = reinterpret_cast<int *>(self + 0x14218);
        for (int k = 0; k < 2; ++k)
            slots[k] = -1;
        return;
    }

    for (int i = 0; i < 0x1D; ++i) {
        unsigned int bit = 1u << i;
        if (!(F14208 & bit))
            continue;

        if ((bit == 0x80 && (F14208 & 0x40))
            || !abil_cond(i, F141FC, F14200, F14204)
            || (bit_count(F14208) > 1
                && (bit_count(F14208) != 2 || !has_tech(*g_00949780, F141F4)))) {
            F14208 &= ~bit;
            int *slots = reinterpret_cast<int *>(self + 0x14218);
            for (int k = 0; k < 2; ++k)
                slots[k] = -1;
            for (int j = 0; j < 0x1D; ++j) {
                if (F14208 & (1u << j)) {
                    for (int k = 0; k < 2; ++k) {
                        if (slots[k] < 0) {
                            F14218(k) = j;
                            break;
                        }
                    }
                }
            }
        }
    }

#undef F141FC
#undef F14200
#undef F14204
#undef F14208
#undef F14218
#undef F141F4
}
