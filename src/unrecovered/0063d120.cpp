// ORIGINAL: 0x0063D120 FILE
// working copy - scaffold materialised by --work
// name      sub_63d120
// size      115 bytes
// spans     0x0063D120-0x0063D193
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063D560 0x00640BB0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63d120  at 0x0063D120  (115 bytes)
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
extern "C" int __cdecl sub_63d560();
extern "C" int __cdecl sub_640bb0();
extern "C" int __cdecl sub_63d120(int param_1) {
    typedef void (__cdecl *Func1)(int);
    typedef void (__cdecl *Func2)(int);

    int field1c;
    if (param_1 == 0 || (field1c = *(int *)(param_1 + 0x1c)) == 0 ||
        *(int *)(param_1 + 0x20) == 0 || *(int *)(param_1 + 0x24) == 0) {
        return -2;
    }

    *(int *)(param_1 + 0x14) = 0;
    *(int *)(param_1 + 8) = 0;
    *(int *)(param_1 + 0x18) = 0;
    *(int *)(param_1 + 0x2c) = 2;
    *(int *)(field1c + 0x10) = 0;
    *(int *)(field1c + 0xc) = *(int *)(field1c + 8);
    if (*(int *)(field1c + 0x14) < 0) {
        *(int *)(field1c + 0x14) = 0;
    }
    unsigned int masked;
    if (*(int *)(field1c + 0x14) != 0) {
        masked = 0x47;
    } else {
        masked = 0;
    }
    *(unsigned int *)(field1c + 4) = masked + 0x2a;
    *(int *)(param_1 + 0x30) = 1;
    *(int *)(field1c + 0x1c) = 0;
    ((Func1)sub_640bb0)(field1c);
    ((Func2)sub_63d560)(field1c);
    return 0;
}
