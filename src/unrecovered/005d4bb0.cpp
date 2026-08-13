// ORIGINAL: 0x005D4BB0 FILE
// working copy - scaffold materialised by --work
// name      sub_5d4bb0
// size      145 bytes
// spans     0x005D4BB0-0x005D4C41
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d4bb0  at 0x005D4BB0  (145 bytes)
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
extern "C" int __stdcall sub_5d4bb0(int a1, int a2) {
    unsigned int *p1 = (unsigned int *)a1;
    unsigned int *p2 = (unsigned int *)a2;
    if (*p1 < *p2) {
        return -1;
    }
    if (*p1 != *p2) {
        return 1;
    }
    unsigned short *s1 = (unsigned short *)(a1 + 4);
    unsigned short *s2 = (unsigned short *)(a2 + 4);
    if (*s1 < *s2) {
        return -1;
    }
    if (*s1 != *s2) {
        return 1;
    }
    unsigned short *t1 = (unsigned short *)(a1 + 6);
    unsigned short *t2 = (unsigned short *)(a2 + 6);
    if (*t1 < *t2) {
        return -1;
    }
    if (*t1 != *t2) {
        return 1;
    }
    unsigned char *b1 = (unsigned char *)(a1 + 8);
    unsigned char *b2 = (unsigned char *)(a2 + 8);
    int i;
    for (i = 0; i < 8; i++) {
        if (b1[i] < b2[i]) {
            return -1;
        }
        if (b1[i] != b2[i]) {
            return 1;
        }
    }
    return 0;
}
