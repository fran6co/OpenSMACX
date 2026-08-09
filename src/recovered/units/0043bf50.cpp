// ORIGINAL: 0x0043BF50 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0043BF50
// name           ?construct_vehicle@DesignWin@@QAEXXZ
// size           274 bytes
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043bf50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?construct_vehicle@DesignWin@@QAEXXZ  at 0x0043BF50  (274 bytes)
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

class Caviar;
class DesignWin;

class Caviar { public:
    float scene_scale_;
    uint8_t unmapped_04_[0x2C - 0x04];
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint8_t unmapped_38_[0x108 - 0x38];
    int32_t field_108_;
    uint8_t unmapped_10C_[0x13D0 - 0x10C];
};

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl vehdraw_construct_abil(int, Caviar *, int);
int __cdecl vehdraw_construct_armor(int, int, int, int, int, Caviar *);
int __cdecl vehdraw_construct_chassis(int, int, int, int, int, Caviar *);
int __cdecl vehdraw_construct_reactor(int, int, int, int, int, Caviar *);
int __cdecl vehdraw_construct_weapon(int, int, int, Caviar *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00733974 = (int *)0x00733974;
static int *const g_00939284 = (int *)0x00939284;

class DesignWin { public:
    void construct_vehicle();
};

// indexed table base: register-scaled index (edx <<4) added to the
// literal - the address itself does work, so it needs extern, not a
// folded pointer constant.
extern int8_t g_0094F280[];

void DesignWin::construct_vehicle() {
    // Reach fields by offset - the class is deliberately empty.
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int32_t *>(self + 0x13b68) = 0;

    int chassis_kind = *reinterpret_cast<int32_t *>(self + 0x14200);
    int saved_141fc = *reinterpret_cast<int32_t *>(self + 0x141fc);

    if (chassis_kind == 0x15 && saved_141fc == 1) {
        int idx = *reinterpret_cast<int32_t *>(self + 0x14204);
        if (g_0094F280[idx * 0x10] <= 1) {
            *reinterpret_cast<int32_t *>(self + 0x141fc) = 0;
        }
    }

    if (*reinterpret_cast<int32_t *>(self + 0x141f8) == 0x14) {
        *reinterpret_cast<int32_t *>(self + 0x141fc) = 0;
    }

    Caviar *chassis_obj = reinterpret_cast<Caviar *>(self + 0x1279c);

    vehdraw_construct_chassis(
        *reinterpret_cast<int32_t *>(self + 0x141f4),
        *reinterpret_cast<int32_t *>(self + 0x141fc),
        chassis_kind,
        *reinterpret_cast<int32_t *>(self + 0x14204),
        *reinterpret_cast<int32_t *>(self + 0x1420c),
        chassis_obj);

    *g_00733974 = 1;

    vehdraw_construct_weapon(
        *reinterpret_cast<int32_t *>(self + 0x141f4),
        *reinterpret_cast<int32_t *>(self + 0x141fc),
        *reinterpret_cast<int32_t *>(self + 0x14200),
        chassis_obj);

    *g_00733974 = 0;

    vehdraw_construct_armor(
        *g_00939284,
        *reinterpret_cast<int32_t *>(self + 0x141fc),
        *reinterpret_cast<int32_t *>(self + 0x14200),
        *reinterpret_cast<int32_t *>(self + 0x14204),
        0,
        chassis_obj);

    vehdraw_construct_reactor(
        *g_00939284,
        *reinterpret_cast<int32_t *>(self + 0x141fc),
        *reinterpret_cast<int32_t *>(self + 0x14200),
        *reinterpret_cast<int32_t *>(self + 0x1420c),
        *reinterpret_cast<int32_t *>(self + 0x14208),
        chassis_obj);

    vehdraw_construct_abil(
        *reinterpret_cast<int32_t *>(self + 0x14208),
        chassis_obj,
        *reinterpret_cast<int32_t *>(self + 0x141f4));

    *reinterpret_cast<int32_t *>(self + 0x141fc) = saved_141fc;
}
