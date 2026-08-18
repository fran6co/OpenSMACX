// ORIGINAL: 0x004C3800 ?vehdraw_construct_reactor@@YAHHHHHHPAUCaviar@@@Z 0x004C3800-0x004C3896 FILE
// working copy - scaffold materialised by --work
// size      150 bytes
// prototype 
// callers   5   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00617920 0x00617F20

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?vehdraw_construct_reactor@@YAHHHHHHPAUCaviar@@@Z  at 0x004C3800  (150 bytes)
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
class CaviarData;
struct VOX_Vect;

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    int add_object(CaviarData *, VOX_Vect *, int);
    int add_object(CaviarData *, int);
};

class CaviarData { public:
    uint32_t field_0_;
    uint32_t fileDescriptor_;
    void * record_;
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00687ae8 = (int *)0x00687AE8;
static int *const g_0076e554 = (int *)0x0076E554;
static int *const g_0076e9a4 = (int *)0x0076E9A4;
static int *const g_00779560 = (int *)0x00779560;
static int *const g_0079961c = (int *)0x0079961C;
extern unsigned char g_0094ae6a[];

int __cdecl vehdraw_construct_reactor(int a1, int a2, int a3, int a4, int a5, Caviar * a6) {
    int result = 0;
    if (a4 != 0) {
        if (a2 == 0) {
            unsigned char kind = g_0094ae6a[a3 * 16];
            if (kind < 3 || kind == 0xb) {
                return a6->add_object((CaviarData *)((char *)g_0079961c + a4 * 12), (VOX_Vect *)g_00687ae8, 0);
            }
        } else if (a2 == 8) {
            return a6->add_object((CaviarData *)((char *)g_0076e9a4 + a4 * 12), 0);
        }
        result = a6->add_object((CaviarData *)((char *)g_0076e554 + a4 * 12), 0);
        if ((a5 & 0x8000) != 0) {
            result = a6->add_object((CaviarData *)g_00779560, 0);
        }
    }
    return result;
}
