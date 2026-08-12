// ORIGINAL: 0x00618DA0 FILE
// name      ?object_start@Caviar@@QAAXPAUVOX_ScnRenderHnd@@JPAUVOX_Vect@@PAUVOX_Matrix@@PAUVOX_Rect@@PAUVOX_Vect@@@Z
// size      102 bytes
// spans     0x00618DA0-0x00618E06
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645930
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00618DA0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00618da0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?object_start@Caviar@@QAAXPAUVOX_ScnRenderHnd@@JPAUVOX_Vect@@PAUVOX_Matrix@@PAUVOX_Rect@@PAUVOX_Vect@@@Z  at 0x00618DA0  (102 bytes)
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
struct VOX_Matrix;
struct VOX_Rect;
struct VOX_ScnRenderHnd;
struct VOX_Vect;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void *memcpy(void *, const void *, unsigned int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b9c38 = (int *)0x009B9C38;
static int *const g_009bb470 = (int *)0x009BB470;
static int *const g_009bb474 = (int *)0x009BB474;

class Caviar { public:
    float scene_scale_;
    uint8_t unmapped_04_[0x2C - 0x04];
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint8_t unmapped_38_[0x108 - 0x38];
    int32_t field_108_;
    uint8_t unmapped_10C_[0x13D0 - 0x10C];

    void __cdecl object_start(VOX_ScnRenderHnd *, long, VOX_Vect *, VOX_Matrix *, VOX_Rect *, VOX_Vect *);
};

// A direct `memcpy(...)` call with a compile-time-constant length is an
// MSVC intrinsic candidate: /O2 replaces it with an inlined `rep movsd`
// (confirmed empirically - a plain named call compiled to exactly that,
// pulling in a frame and esi/edi pushes the original never has). The
// original is a real `call`, so this goes through a pointer to defeat the
// by-name recognition; callee TYPE still matches the catalog exactly.
static void *(__cdecl *const memcpy_fn)(void *, const void *, unsigned int) = memcpy;

void __cdecl Caviar::object_start(VOX_ScnRenderHnd * a1, long a2, VOX_Vect * a3, VOX_Matrix * a4, VOX_Rect * a5, VOX_Vect * a6) {
    char *base = reinterpret_cast<char *>(*g_009bb470);
    if (*reinterpret_cast<unsigned char *>(base + 4) & 8) {
        void *dest = *reinterpret_cast<void **>(reinterpret_cast<char *>(a1) + 0x50);
        memcpy_fn(dest, g_009b9c38, 0x1800);
        return;
    }
    if (*reinterpret_cast<unsigned char *>(base + 4) & 0x10) {
        void *dest = *reinterpret_cast<void **>(reinterpret_cast<char *>(a1) + 0x50);
        memcpy_fn(dest, reinterpret_cast<void *>(*g_009bb474), 0x1800);
        return;
    }
    void *dest = *reinterpret_cast<void **>(reinterpret_cast<char *>(a1) + 0x50);
    void *src = *reinterpret_cast<void **>(base + 0x10ac + a2 * 4);
    memcpy_fn(dest, src, 0x1800);
}
