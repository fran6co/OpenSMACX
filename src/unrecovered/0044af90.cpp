// ORIGINAL: 0x0044AF90 sub_44af90 0x0044AF90-0x0044AFF2 FILE BYTE_EXACT
// LEVER: write-branch-first if/else polarity (matches the original's `je` target) plus a signed (int) pointer-vs-address comparison instead of unsigned, in the zero-fill loop's exit test
// working copy - scaffold materialised by --work
// size      98 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064603F 0x00646178

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_44af90  at 0x0044AF90  (98 bytes)
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
extern "C" int __cdecl _fread(void*, size_t, size_t, FILE*);
extern "C" int __cdecl _fwrite(void*, size_t, size_t, FILE*);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00759194 = (int *)0x00759194;
static int *const g_009a3840 = (int *)0x009A3840;
static int *const g_009a4b64 = (int *)0x009A4B64;
extern "C" int __cdecl sub_44af90(FILE *a1, int a2) {
    *g_00759194 = a2;
    int result;
    if (a2 != 0) {
        result = _fwrite(g_009a3840, 0x1324, 1, a1);
    } else {
        result = _fread(g_009a3840, 0x1324, 1, a1);
    }
    if (result == 0) {
        uint8_t *p = (uint8_t *)g_009a3840;
        do {
            int i = 5;
            do {
                *p = 0;
                p += 0x8c;
                i--;
            } while (i != 0);
        } while ((int)p < (int)g_009a4b64);
    }
    return 0;
}
