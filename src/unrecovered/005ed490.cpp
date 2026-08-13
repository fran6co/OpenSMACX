// ORIGINAL: 0x005ED490 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      sub_5ed490
// size      137 bytes
// spans     0x005ED490-0x005ED519
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED490

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ed490  at 0x005ED490  (137 bytes)
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
extern "C" void __cdecl sub_5ed490(int *a1, int *a2, int a3) {
    *a1 -= *(int *)(a3 + 0x14c) + *(int *)(a3 + 0x13c);
    *a2 -= *(int *)(a3 + 0x150) + *(int *)(a3 + 0x140);
    if ((*(unsigned char *)(a3 + 0x98) & 0x20) != 0 && *(int *)(a3 + 0xc4) != 0) {
        sub_5ed490(a1, a2, *(int *)(a3 + 0xc4));
        if ((*(unsigned int *)(a3 + 0x98) & 0x8000) != 0) {
            *a1 += *(int *)(*(int *)(a3 + 0xc4) + 0x13c);
            *a2 += *(int *)(*(int *)(a3 + 0xc4) + 0x140);
        }
    }
}
