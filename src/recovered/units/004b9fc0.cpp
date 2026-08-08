// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004B9FC0
// name           ?init@StatusWin@@QAEXXZ
// size           350 bytes
// measured tier  MISMATCH
// divergence     33
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004b9fc0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@StatusWin@@QAEXXZ  at 0x004B9FC0  (350 bytes)
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
class Spot;
class StatusWin;
struct VOX_Matrix;
struct VOX_Vect;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Caviar { public:
    float scene_scale_;
    uint8_t unmapped_04_[0x2C - 0x04];
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint8_t unmapped_38_[0x108 - 0x38];
    int32_t field_108_;
    uint8_t unmapped_10C_[0x13D0 - 0x10C];
    void init();
    void set_camera_direct(VOX_Vect *, VOX_Matrix *);
    void set_scaling(float);
};
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    void init(int);
};
extern "C" int __cdecl sub_627d00();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066aa40 = (int *)0x0066AA40;
static int *const g_00939ed8 = (int *)0x00939ED8;

class StatusWin { public:
    void init();
};

// The RMW below the flag test needs a plain global, not the const-pointer
// spelling above: `*g_00939ed8 |= 0x20` through the const pointer compiles
// to load/modify/store (extra mov, extra mov), where a genuine `extern int`
// object lets /O2 fold the OR straight into a memory operand.
extern int g_00939ed8_ext;

void StatusWin::init() {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int *>(self + 0x15a8) = 0;
    *reinterpret_cast<int *>(self + 0x15d8) = 0;
    *reinterpret_cast<int *>(self + 0x15b4) = 0;
    *reinterpret_cast<int *>(self + 0x15b8) = 0;
    *reinterpret_cast<int *>(self + 0x15bc) = -1;
    *reinterpret_cast<int *>(self + 0x15c0) = -1;
    *reinterpret_cast<int *>(self + 0x15c8) = -1;
    *reinterpret_cast<int *>(self + 0x15c4) = -1;
    *reinterpret_cast<int *>(self + 0x15d0) = -1;
    *reinterpret_cast<int *>(self + 0x15cc) = -1;
    *reinterpret_cast<int *>(self + 0x15ac) = -1;
    *reinterpret_cast<int *>(self + 0x15d4) = 0;

    unsigned char statusFlags = *reinterpret_cast<unsigned char *>(&g_00939ed8_ext);
    if ((statusFlags & 0x20) == 0) {
        g_00939ed8_ext |= 0x20;
    }

    reinterpret_cast<Spot *>(self + 0x15e0)->init(0x18);

    Caviar *caviar = reinterpret_cast<Caviar *>(self + 0x30);
    caviar->init();

    *reinterpret_cast<char *>(self + 0xd4) = 0;
    *reinterpret_cast<int *>(self + 0x1504) = 0;
    *reinterpret_cast<unsigned int *>(self + 0x1508) = 0x3f48f5c3;
    *reinterpret_cast<int *>(self + 0x1500) = 0;

    // The catalogued declaration for this callee is `sub_627d00()` (zero
    // args); the call goes through a function-pointer cast to the real
    // two-argument prototype instead of redeclaring the callee.
    void *matrix = self + 0x150c;
    reinterpret_cast<int (__cdecl *)(void *, void *)>(sub_627d00)(
        self + 0x1500, matrix);

    float negX = -*reinterpret_cast<float *>(self + 0x1524);
    *reinterpret_cast<float *>(self + 0x1524) = negX;
    float negY = -*reinterpret_cast<float *>(self + 0x1528);
    *reinterpret_cast<float *>(self + 0x1528) = negY;
    float negZ = -*reinterpret_cast<float *>(self + 0x152c);
    *reinterpret_cast<float *>(self + 0x152c) = negZ;

    float direction[3];
    direction[0] = negX * *reinterpret_cast<float *>(g_0066aa40);
    direction[1] = negY * *reinterpret_cast<float *>(g_0066aa40);
    direction[2] = negZ * *reinterpret_cast<float *>(g_0066aa40);

    caviar->set_camera_direct(reinterpret_cast<VOX_Vect *>(direction),
                              reinterpret_cast<VOX_Matrix *>(matrix));
    caviar->set_scaling(0.18f);

    *reinterpret_cast<int *>(self + 0x9c) = 0;
    *reinterpret_cast<int *>(self + 0x90) = 0;
    *reinterpret_cast<int *>(self + 0xa0) = 0;
    *reinterpret_cast<int *>(self + 0x94) = 0;
    *reinterpret_cast<unsigned int *>(self + 0xa4) = 0x3f800000;
    *reinterpret_cast<unsigned int *>(self + 0x98) = 0x3f800000;
}
