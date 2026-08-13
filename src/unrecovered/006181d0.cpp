// ORIGINAL: 0x006181D0 FILE
// working copy - scaffold materialised by --work
// name      ?set_shadow_vector@Caviar@@QAEXMM@Z
// size      123 bytes
// spans     0x006181D0-0x0061824B
// prototype void (__thiscall ?set_shadow_vector@Caviar@@QAEXMM@Z)(Caviar* this, float, float)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006463E4 0x00646494

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_shadow_vector@Caviar@@QAEXMM@Z  at 0x006181D0  (123 bytes)
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

class Caviar { public:
    float scene_scale_;
    uint32_t field_4_;
    uint8_t field_8_[0xC];
    uint32_t field_14_;
    uint8_t field_18_[0x4];
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    float field_6C_;
    float field_70_;
    uint32_t field_74_;
    uint32_t field_78_;
    uint32_t field_7C_;
    uint32_t field_80_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    uint8_t field_A4_;
    uint8_t field_A5_[0x63];
    int32_t field_108_;
    uint8_t field_10C_[0x640];
    uint32_t field_74C_;
    uint32_t field_750_;
    uint32_t field_754_;
    uint8_t field_758_[0xC74];
    uint32_t field_13CC_;

    void set_shadow_vector(float, float);
};
extern "C" double __cdecl c_sin(double);
extern "C" double __cdecl c_cos(double);

void Caviar::set_shadow_vector(float a1, float a2) {
    char *self = reinterpret_cast<char *>(this);
    float s1 = static_cast<float>(c_sin(a1));
    float s2 = static_cast<float>(c_sin(a2));
    float c1 = static_cast<float>(c_cos(a1));
    float c2 = static_cast<float>(c_cos(a2));
    *reinterpret_cast<float *>(self + 0x74) = c1;
    *reinterpret_cast<float *>(self + 0x6c) = c2 * s1;
    *reinterpret_cast<float *>(self + 0x70) = s2 * s1;
}
