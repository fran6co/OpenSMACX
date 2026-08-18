// ORIGINAL: 0x004D5F70 sub_4d5f70 0x004D5F70-0x004D5FE6 FILE
// working copy - scaffold materialised by --work
// size      118 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00628A50

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4d5f70  at 0x004D5F70  (118 bytes)
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
void swap(int *, int *);

// Plain cdecl args, no ecx/this use; a bubble-sort style pass over an int
// array (a2, a3 elements) with a parallel array based at a1.
void __cdecl sub_4d5f70(int a1, int *a2, int a3) {
    int limit = a3 - 1;
    int swapped;
    do {
        swapped = 0;
        if (limit < 1) {
            return;
        }
        int p1 = a1;
        int *p2 = a2;
        a3 = limit;
        do {
            int *next = p2 + 1;
            if (*p2 < *next) {
                swapped = 1;
                swap(p2, next);
                swap(reinterpret_cast<int *>(p1),
                     reinterpret_cast<int *>((a1 - reinterpret_cast<int>(a2)) + reinterpret_cast<int>(next)));
            }
            p1 = p1 + 4;
            a3 = a3 - 1;
            p2 = next;
        } while (a3 != 0);
    } while (swapped != 0);
}
