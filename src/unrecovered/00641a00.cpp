// ORIGINAL: 0x00641A00 FILE
// working copy - scaffold materialised by --work
// name      sub_641a00
// size      111 bytes
// spans     0x00641A00-0x00641A6F
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006414B0 0x00641A70

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_641a00  at 0x00641A00  (111 bytes)
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
extern "C" int __cdecl build_tree();
extern "C" int __cdecl sub_641a70();
static unsigned char *const g_00698fe0 = (unsigned char *)0x00698FE0;

// `mov esi, [esp+8]` reads the CALLER's stack arg, not `ecx` -> a plain
// __cdecl parameter, not a missing `this`.
extern "C" int __cdecl sub_641a00(int a1) {
    char *self = reinterpret_cast<char *>(a1);
    typedef int (__cdecl *Fn3)(int, int, int);
    typedef int (__cdecl *Fn2)(int, int);
    Fn3 fn70 = reinterpret_cast<Fn3>(sub_641a70);
    Fn2 fnTree = reinterpret_cast<Fn2>(build_tree);

    fn70(a1, a1 + 0x88, *reinterpret_cast<int *>(self + 0xb10));
    fn70(a1, a1 + 0x97c, *reinterpret_cast<int *>(self + 0xb1c));
    fnTree(a1, a1 + 0xb24);

    int i = 0x12;
    do {
        int idx = g_00698fe0[i];
        if (*reinterpret_cast<short *>(self + 0xa72 + idx * 4) != 0) {
            break;
        }
        i--;
    } while (i >= 3);

    int *counter = reinterpret_cast<int *>(self + 0x169c);
    *counter += i * 3 + 0x11;
    return 0;
}
