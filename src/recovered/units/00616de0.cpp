// ORIGINAL: 0x00616DE0 FILE
// name      ?init@Caviar@@QAEXXZ
// size      563 bytes
// spans     0x00616DE0-0x00617013
// prototype void (__thiscall ?init@Caviar@@QAEXXZ)(Caviar* this)
// callers   7   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00617020 0x00628220 0x006463E4 0x00646494
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00616DE0
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00616de0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@Caviar@@QAEXXZ  at 0x00616DE0  (563 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl cos();
extern "C" int __cdecl sin();
extern "C" int __cdecl sub_628220();
void * __cdecl mem_get(int);

class Caviar { public:
    float scene_scale_;
    uint8_t unmapped_04_[0x2C - 0x04];
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint8_t unmapped_38_[0x108 - 0x38];
    int32_t field_108_;
    uint8_t unmapped_10C_[0x13D0 - 0x10C];

    void close();
    void init();
};

// sin/cos/sub_628220 are emitted nullary (arity unknown to the catalogue),
// but the call sites here clearly pass a double / two pointers, visible from
// the pushed args and the `add esp, 0x28` cleanup after sub_628220. Declaring
// a second, differently-typed extern "C" overload of the same name is
// `error C2733: second C linkage`, so the real signature is applied by taking
// the ALREADY-DECLARED symbol's address and reinterpreting the function
// pointer type, not by re-declaring the name.
typedef double(__cdecl *SinFn)(double);
typedef double(__cdecl *CosFn)(double);
typedef void(__cdecl *Sub628220Fn)(void *, void *);
static const SinFn real_sin = reinterpret_cast<SinFn>(&sin);
static const CosFn real_cos = reinterpret_cast<CosFn>(&cos);
static const Sub628220Fn real_sub_628220 = reinterpret_cast<Sub628220Fn>(&sub_628220);

void Caviar::init() {
    char *self = reinterpret_cast<char *>(this);
    close();

    int count = 0;
    char *slot = self + 0x750;
    for (;;) {
        void *p1 = mem_get(1);
        *reinterpret_cast<void **>(slot - 4) = p1;
        if (p1 == 0) {
            close();
            return;
        }
        void *p2 = mem_get(0xc);
        *reinterpret_cast<void **>(slot) = p2;
        if (p2 == 0) {
            close();
            return;
        }
        void *p3 = mem_get(0x24);
        *reinterpret_cast<void **>(slot + 4) = p3;
        if (p3 == 0) {
            close();
            return;
        }
        ++count;
        slot += 0xc;
        if (count >= 200) {
            break;
        }
    }

    *reinterpret_cast<int *>(self + 0x13cc) = 0;
    field_108_ = 0x42c80000;

    float sin1 = static_cast<float>(real_sin(1.0));
    float sin0 = static_cast<float>(real_sin(0.0));
    float cos1 = static_cast<float>(real_cos(1.0));
    float prod1 = static_cast<float>(real_cos(0.0) * sin1);

    *reinterpret_cast<float *>(self + 0x6c) = prod1;
    float prod2 = static_cast<float>(sin0 * sin1);

    field_2C_ = 0;
    field_30_ = 0;
    field_34_ = 0;
    *reinterpret_cast<float *>(self + 0x5c) = 1.0f;

    *reinterpret_cast<float *>(self + 0x70) = prod2;
    *reinterpret_cast<float *>(self + 0x74) = cos1;
    *reinterpret_cast<float *>(self + 0xd5) = 1.0f;
    *reinterpret_cast<int *>(self + 0xe5) = 0;
    *reinterpret_cast<unsigned int *>(self + 0xe9) = 0x477fff00u;
    *reinterpret_cast<int *>(self + 0xed) = 0;
    *reinterpret_cast<int *>(self + 0xf1) = 0;
    *reinterpret_cast<int *>(self + 0xf5) = 0xff;
    *reinterpret_cast<int *>(self + 0xf9) = 0xff;
    *reinterpret_cast<int *>(self + 0xfd) = 0x100;
    *reinterpret_cast<int *>(self + 0x101) = 0x100;
    *reinterpret_cast<int *>(self + 0x7c) = 0;
    *reinterpret_cast<int *>(self + 0x80) = 0;
    *reinterpret_cast<int *>(self + 0x84) = 0;
    *reinterpret_cast<int *>(self + 0x88) = 0;
    *reinterpret_cast<int *>(self + 0x8c) = 0;
    *reinterpret_cast<float *>(self + 0x90) = 1.0f;
    *reinterpret_cast<int *>(self + 0x94) = 0;
    *reinterpret_cast<int *>(self + 0x98) = 0;
    *reinterpret_cast<float *>(self + 0x9c) = 1.0f;
    *reinterpret_cast<float *>(self + 0xa0) = 1.0f;
    *reinterpret_cast<int *>(self + 0xd9) = 0;
    *reinterpret_cast<unsigned int *>(self + 0xdd) = 0xc77fff00u;
    *reinterpret_cast<unsigned int *>(self + 0xe1) = 0x477fff00u;
    *reinterpret_cast<int *>(self + 0x1c) = 0;
    *reinterpret_cast<void **>(self + 0x24) = self + 0x74c;
    *reinterpret_cast<int *>(self + 0x20) = 0;
    *reinterpret_cast<void **>(self + 0x28) = self + 0x10c;

    *reinterpret_cast<int *>(self + 0x60) = 0;
    *reinterpret_cast<int *>(self + 0x64) = 0;
    *reinterpret_cast<int *>(self + 0x68) = 0;
    *reinterpret_cast<int *>(self + 0x60) = 0;
    *reinterpret_cast<int *>(self + 0x64) = 0;
    *reinterpret_cast<float *>(self + 0x68) = -1.0f;

    real_sub_628220(self + 0x60, self + 0x60);

    *reinterpret_cast<float *>(self + 0x38) = 1.0f;
    *reinterpret_cast<int *>(self + 0x3c) = 0;
    float rx = *reinterpret_cast<float *>(self + 0x60);
    float ry = *reinterpret_cast<float *>(self + 0x64);
    float rz = *reinterpret_cast<float *>(self + 0x68);
    *reinterpret_cast<float *>(self + 0x6c) = rx;
    *reinterpret_cast<float *>(self + 0x70) = ry;

    unsigned int flags = *reinterpret_cast<unsigned int *>(self + 4);
    flags |= 1;

    *reinterpret_cast<int *>(self + 0x40) = 0;
    *reinterpret_cast<int *>(self + 0x44) = 0;
    *reinterpret_cast<int *>(self + 0x4c) = 0;
    *reinterpret_cast<int *>(self + 0x50) = 0;
    *reinterpret_cast<int *>(self + 0x54) = 0;
    *reinterpret_cast<int *>(self + 0xb5) = 0;
    *reinterpret_cast<int *>(self + 0xb9) = 0;
    *reinterpret_cast<int *>(self + 0xbd) = 0;
    *reinterpret_cast<int *>(self + 0xc5) = 0;
    *reinterpret_cast<int *>(self + 0xc9) = 0;
    *reinterpret_cast<int *>(self + 0xcd) = 0;
    *reinterpret_cast<int *>(self + 0x78) = 0;
    *reinterpret_cast<float *>(self + 0x48) = 1.0f;
    *reinterpret_cast<float *>(self + 0x58) = 1.0f;
    *reinterpret_cast<float *>(self + 0xb1) = 1.0f;
    *reinterpret_cast<float *>(self + 0xc1) = 1.0f;
    *reinterpret_cast<float *>(self + 0xd1) = 1.0f;
    *reinterpret_cast<unsigned char *>(self + 0xa4) = 1;
    *reinterpret_cast<float *>(self + 0x74) = rz;
    *reinterpret_cast<unsigned int *>(self + 4) = flags;
}
