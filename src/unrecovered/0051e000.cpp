// ORIGINAL: 0x0051E000 ?read_tamper@@YAXXZ 0x0051E000-0x0051E087 FILE
// working copy - scaffold materialised by --work
// size      135 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0051E026 0x0051E055 0x0051E076 0x0051E07C

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?read_tamper@@YAXXZ  at 0x0051E000  (135 bytes)
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
static int *const g_00669000 = (int *)0x00669000;
static int *const g_00669004 = (int *)0x00669004;
static int *const g_00669010 = (int *)0x00669010;
static int *const g_0068ad88 = (int *)0x0068AD88;
static int *const g_0068add0 = (int *)0x0068ADD0;
static int *const g_0068add4 = (int *)0x0068ADD4;
static int *const g_0093a060 = (int *)0x0093A060;
static int *const g_0093a9f8 = (int *)0x0093A9F8;
static int *const g_0093a9fc = (int *)0x0093A9FC;
typedef int (__stdcall *RegCreateKeyExA_fn)(int, const char *, int, void *, int, int, void *, int *, void *);
typedef int (__stdcall *RegQueryValueExA_fn)(int, const char *, void *, int *, void *, int *);
typedef int (__stdcall *RegCloseKey_fn)(int);

void __cdecl read_tamper() {
    int hKey;
    int dwType;
    int cbData;

    int status = (*reinterpret_cast<RegCreateKeyExA_fn *>(g_00669004))(
        0x80000002, reinterpret_cast<const char *>(g_0068ad88), 0, g_0093a9fc, 0, 0xf003f, 0, &hKey, 0);
    if (status == 0) {
        cbData = 4;
        (*reinterpret_cast<RegQueryValueExA_fn *>(g_00669000))(
            hKey, reinterpret_cast<const char *>(g_0068add0), 0, &dwType, g_0093a9f8, &cbData);
        cbData = 0x4b0;
        (*reinterpret_cast<RegQueryValueExA_fn *>(g_00669000))(
            hKey, reinterpret_cast<const char *>(g_0068add4), 0, &dwType, g_0093a060, &cbData);
        (*reinterpret_cast<RegCloseKey_fn *>(g_00669010))(hKey);
    }
}
