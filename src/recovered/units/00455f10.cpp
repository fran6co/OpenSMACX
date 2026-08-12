// ORIGINAL: 0x00455F10 FILE
// name      ??0FactionArt@@QAE@XZ
// size      412 bytes
// spans     0x00455F10-0x0045600B;0x00654ED0-0x00654F71
// prototype FactionArt* (__thiscall ??0FactionArt@@QAE@XZ)(FactionArt* this)
// callers   0   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005E37E0 0x006456E4 0x006457C2
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00455F10
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00455f10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0FactionArt@@QAE@XZ  at 0x00455F10  (412 bytes)
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

class FactionArt;
class Sprite;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;
    Sprite();
};
void __stdcall fn_006456e4(void*, unsigned int, int, void (*)(void*));
void __stdcall fn_006457c2(void*, unsigned int, int, void (*)(void*), void (*)(void*));

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00406850 = (int *)0x00406850;
static int *const g_005e37e0 = (int *)0x005E37E0;
static int *const g_00654f67 = (int *)0x00654F67;
static int *const g_006751d0 = (int *)0x006751D0;

class FactionArt { public:
    FactionArt();
};

inline void *operator new(unsigned int, void *p) { return p; }

// A real `Sprite arr[N];` array member compiles to a call to `??_H`
// (the plain, non-EH vector-constructor-iterator helper); the original
// calls `??_L` (fn_006457c2), the EH-aware one, directly - matching
// AlphaSave::AlphaSave (0x00407350), which hits the same shape and
// resolves it the same way: explicit fn_006457c2 calls for the array
// runs, placement-new for the scalars between them, one call per
// original call, in the same order. Offsets and counts read off the
// disassembly:
//   0x000 Sprite[24]  (24*0x2c = 0x420)
//   0x420 Sprite[2]   (2*0x2c  = 0x58)
//   0x478 Sprite      single1
//   0x4a4 Sprite      single2
//   0x4d0 Sprite      single3
//   0x4fc Sprite      single4
//   0x528 Sprite      single5
//   0x554 Sprite      single6
//   0x580 Sprite[3]   (3*0x2c  = 0x84)
//   0x604 Sprite[2]   (2*0x2c  = 0x58, ends at 0x65c)
FactionArt::FactionArt() {
    char *self = reinterpret_cast<char *>(this);

    fn_006457c2(self, 0x2c, 0x18,
                reinterpret_cast<void (*)(void *)>(g_005e37e0),
                reinterpret_cast<void (*)(void *)>(g_00406850));
    fn_006457c2(self + 0x420, 0x2c, 2,
                reinterpret_cast<void (*)(void *)>(g_005e37e0),
                reinterpret_cast<void (*)(void *)>(g_00406850));
    new (self + 0x478) Sprite();
    new (self + 0x4a4) Sprite();
    new (self + 0x4d0) Sprite();
    new (self + 0x4fc) Sprite();
    new (self + 0x528) Sprite();
    new (self + 0x554) Sprite();
    fn_006457c2(self + 0x580, 0x2c, 3,
                reinterpret_cast<void (*)(void *)>(g_005e37e0),
                reinterpret_cast<void (*)(void *)>(g_00406850));
    fn_006457c2(self + 0x604, 0x2c, 2,
                reinterpret_cast<void (*)(void *)>(g_005e37e0),
                reinterpret_cast<void (*)(void *)>(g_00406850));
}
