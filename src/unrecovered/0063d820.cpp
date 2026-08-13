// ORIGINAL: 0x0063D820 FILE
// RULED-OUT: signature changed to (int*, void*, unsigned int) per disasm
//            (3 cdecl stack params); mismatch is in the prologue (#2, push
//            vs mov) - regalloc/param-order difference, not tried further.
// working copy - scaffold materialised by --work
// name      sub_63d820
// size      113 bytes
// spans     0x0063D820-0x0063D891
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00640A80

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63d820  at 0x0063D820  (113 bytes)
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
extern "C" int __cdecl sub_640a80();
typedef int (__cdecl *Sub640a80Fn)(int, int, unsigned int);
extern "C" void *memcpy(void *, const void *, unsigned int);

extern "C" unsigned int __cdecl sub_63d820(int *param_1, void *param_2, unsigned int param_3) {
    unsigned int available = (unsigned int)param_1[1];
    unsigned int count = available;
    if (param_3 < available) {
        count = param_3;
    }
    if (count == 0) {
        return 0;
    }
    param_1[1] = (int)(available - count);
    if (*(int *)(param_1[7] + 0x14) == 0) {
        Sub640a80Fn fn = (Sub640a80Fn)&sub_640a80;
        param_1[0xc] = fn(param_1[0xc], param_1[0], count);
    }
    memcpy(param_2, (void *)param_1[0], count);
    param_1[0] += count;
    param_1[2] += count;
    return count;
}
