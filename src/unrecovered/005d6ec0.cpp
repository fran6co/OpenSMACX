// ORIGINAL: 0x005D6EC0 sub_5d6ec0 0x005D6EC0-0x005D6F4B FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      139 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d6ec0  at 0x005D6EC0  (139 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b3394 = (int *)0x009B3394;
static int *const g_009b3398 = (int *)0x009B3398;
static int *const g_009b339c = (int *)0x009B339C;
static int *const g_009b33a0 = (int *)0x009B33A0;
static int *const g_009b33a4 = (int *)0x009B33A4;
static int *const g_009b33a8 = (int *)0x009B33A8;
static int *const g_009b33ac = (int *)0x009B33AC;
static int *const g_009b33b0 = (int *)0x009B33B0;
static int *const g_009b33b4 = (int *)0x009B33B4;
static int *const g_009b33b8 = (int *)0x009B33B8;
static int *const g_009b33bc = (int *)0x009B33BC;
class Sub5d6ec0Obj {
public:
    void sub_5d6ec0();
};

void Sub5d6ec0Obj::sub_5d6ec0() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<unsigned int *>(self + 0x98) & 0x30000000) {
        *reinterpret_cast<int *>(self + 0x9cc) = *g_009b3394;
        *reinterpret_cast<int *>(self + 0x9d4) = *g_009b339c;
        *reinterpret_cast<int *>(self + 0x9d0) = *g_009b3398;
        *reinterpret_cast<int *>(self + 0x9d8) = *g_009b33a0;
        *reinterpret_cast<int *>(self + 0x9e8) = *g_009b33a4;
        *reinterpret_cast<int *>(self + 0x9e4) = *g_009b33a8;
        *reinterpret_cast<int *>(self + 0x9dc) = *g_009b33ac;
        *reinterpret_cast<int *>(self + 0x9e0) = *g_009b33b0;
        *reinterpret_cast<int *>(self + 0x9ec) = *g_009b33b8;
        *reinterpret_cast<int *>(self + 0x9f0) = *g_009b33b4;
        *reinterpret_cast<int *>(self + 0x9f4) = *g_009b33bc;
    }
}
