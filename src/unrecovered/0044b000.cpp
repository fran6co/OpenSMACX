// ORIGINAL: 0x0044B000 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      ?hof_save@@YAHXZ
// size      108 bytes
// spans     0x0044B000-0x0044B06C
// prototype 
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645598 0x00645646 0x0064603F

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hof_save@@YAHXZ  at 0x0044B000  (108 bytes)
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

struct FILE;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl _fwrite(void*, size_t, size_t, FILE*);
extern "C" int __cdecl fclose();
extern "C" int __cdecl fopen();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068460c = (int *)0x0068460C;
static int *const g_00684610 = (int *)0x00684610;
static int *const g_00759194 = (int *)0x00759194;
static int *const g_009a3840 = (int *)0x009A3840;
static int *const g_009a4b64 = (int *)0x009A4B64;
extern "C" FILE *__cdecl hof_fopen(const char *, const char *);
extern "C" int __cdecl hof_fclose(FILE *);

struct HofRecord {
    char field0_;
    char pad_[0x8B];
};
static HofRecord *const g_hof_records = (HofRecord *)0x9A3840;

int __cdecl hof_save() {
    FILE *file = hof_fopen("halloffame.jhh", "wb");
    if (file == 0) {
        return 1;
    }
    *g_00759194 = 1;
    int written = _fwrite(g_hof_records, 0x1324, 1, file);
    if (written == 0) {
        HofRecord *p = g_hof_records;
        do {
            int n = 5;
            do {
                p->field0_ = 0;
                ++p;
            } while (--n != 0);
        } while (reinterpret_cast<int>(p) < 0x9A4B64);
    }
    hof_fclose(file);
    return 0;
}
