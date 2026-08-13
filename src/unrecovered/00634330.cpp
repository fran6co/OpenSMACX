// ORIGINAL: 0x00634330 FILE
// working copy - scaffold materialised by --work
// name      sub_634330
// size      141 bytes
// spans     0x00634330-0x006343BD
// prototype 
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063DF90 0x0063E0F0 0x0063E110

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634330  at 0x00634330  (141 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl inflate();
extern "C" int __cdecl sub_63df90();
extern "C" int __cdecl sub_63e0f0();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00697f28 = (int *)0x00697F28;
typedef int (__cdecl *Sub63e0f0Fn)(void *, const void *, int);
typedef int (__cdecl *InflateFn)(void *, int);
typedef int (__cdecl *Sub63df90Fn)(void *);

static const Sub63e0f0Fn real_sub_63e0f0 = (Sub63e0f0Fn)&sub_63e0f0;
static const InflateFn real_inflate = (InflateFn)&inflate;
static const Sub63df90Fn real_sub_63df90 = (Sub63df90Fn)&sub_63df90;

extern "C" int __cdecl sub_634330(int p0, int *p1, int p2, int p3) {
    int buffer[14];

    buffer[1] = p3;
    buffer[2] = p0;
    buffer[3] = p2;
    buffer[7] = 0;
    buffer[8] = 0;

    int status = real_sub_63e0f0(buffer, g_00697f28, 0x38);
    if (status != 0) {
        return status;
    }

    int r = real_inflate(buffer, 4);
    if (r != 1) {
        real_sub_63df90(buffer);
        return r;
    }

    *p1 = buffer[5];
    return real_sub_63df90(buffer);
}
