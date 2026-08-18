// ORIGINAL: 0x005ED360 sub_5ed360 0x005ED360-0x005ED3E9 FILE
// RULED-OUT: the given nullary `int __cdecl sub_5ed360()` head - the body reads 3 stack args at [esp+0xc/0x10/0x14] with no ebp frame, so it takes (int*,int*,int) and returns void; recurses on itself. MISMATCH #2 'mov' vs 'push' remains open.
// working copy - scaffold materialised by --work
// size      137 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED360

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ed360  at 0x005ED360  (137 bytes)
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
extern "C" void __cdecl sub_5ed360(int *a1, int *a2, int a3) {
    char *obj = reinterpret_cast<char *>(a3);
    *a1 = *a1 + *reinterpret_cast<int *>(obj + 0x14c) + *reinterpret_cast<int *>(obj + 0x13c);
    *a2 = *a2 + *reinterpret_cast<int *>(obj + 0x150) + *reinterpret_cast<int *>(obj + 0x140);

    unsigned char flags1 = *reinterpret_cast<unsigned char *>(obj + 0x98);
    int child = *reinterpret_cast<int *>(obj + 0xc4);
    if ((flags1 & 0x20) != 0 && child != 0) {
        sub_5ed360(a1, a2, child);
        unsigned int flags2 = *reinterpret_cast<unsigned int *>(obj + 0x98);
        if (flags2 & 0x8000) {
            char *childObj = reinterpret_cast<char *>(child);
            *a1 = *a1 - *reinterpret_cast<int *>(childObj + 0x13c);
            *a2 = *a2 - *reinterpret_cast<int *>(childObj + 0x140);
        }
    }
}
