// ORIGINAL: 0x00620260 FILE
// working copy - scaffold materialised by --work
// name      sub_620260
// size      120 bytes
// spans     0x00620260-0x006202D8
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00620100

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_620260  at 0x00620260  (120 bytes)
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
extern "C" int __cdecl sub_620100();
// `mov eax, [esp+4]` reads the caller's stack arg -> a plain __cdecl
// parameter, not a missing `this`.
extern "C" int __cdecl sub_620260(int a1) {
    char *self = reinterpret_cast<char *>(a1);
    int *pField4 = reinterpret_cast<int *>(self + 4);
    int remaining = *pField4 - 1;
    *pField4 = remaining;
    if (remaining == 0) {
        typedef int (__cdecl *Fn2)(int, int);
        int r = reinterpret_cast<Fn2>(sub_620100)(a1, *reinterpret_cast<int *>(self + 8));
        return r != 0;
    }
    *reinterpret_cast<int *>(self + 0x10) += *reinterpret_cast<int *>(self + 0x18);
    int sum1c = *reinterpret_cast<int *>(self + 0x1c) + *reinterpret_cast<int *>(self + 0x20);
    *reinterpret_cast<int *>(self + 0xc) += *reinterpret_cast<int *>(self + 0x14);
    *reinterpret_cast<int *>(self + 0x1c) = sum1c;
    *reinterpret_cast<int *>(self + 0x34) += *reinterpret_cast<int *>(self + 0x38);
    int sum28 = *reinterpret_cast<int *>(self + 0x28) + *reinterpret_cast<int *>(self + 0x2c);
    *reinterpret_cast<int *>(self + 0x28) = sum28;
    if (sum28 > 0) {
        *reinterpret_cast<int *>(self + 0x1c) = *reinterpret_cast<int *>(self + 0x24) + sum1c;
        *reinterpret_cast<int *>(self + 0x28) = sum28 - *reinterpret_cast<int *>(self + 0x30);
    }
    return 1;
}
