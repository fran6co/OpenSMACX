// ORIGINAL: 0x004C7300 FILE
// RULED-OUT: scaffold had 0 params/cdecl; disassembly reads ecx with no
//            stack access -> changed to `__fastcall sub_4c7300(int
//            param_1)`. Redeclared the callee fn_0064557f (operator
//            delete@0x64557f) to take `void *` instead of the scaffold's
//            0-arg stub. for-loop over the outer bucket scan diverges at
//            instr #24 ('mov' vs rebuilt 'sub').
// working copy - scaffold materialised by --work
// name      sub_4c7300
// size      115 bytes
// spans     0x004C7300-0x004C7373
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0064557F

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c7300  at 0x004C7300  (115 bytes)
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
extern "C" void __cdecl fn_0064557f(void *);
extern "C" int __fastcall sub_4c7300(int param_1) {
    int i;
    for (i = 0; i < 0x80; i = i + 1) {
        int *bucket = *reinterpret_cast<int **>(param_1 + i * 4);
        if (bucket != 0) {
            i = *bucket;
            int *rec = *reinterpret_cast<int **>(param_1 + i * 4);
            unsigned int start = static_cast<unsigned int>(rec[1]);
            unsigned int end = static_cast<unsigned int>(rec[0]) + 1;
            int *slot = reinterpret_cast<int *>(param_1 + start * 4);
            fn_0064557f(reinterpret_cast<void *>(*slot));
            if (start < end) {
                unsigned int count = end - start;
                while (count != 0) {
                    *slot = 0;
                    slot = slot + 1;
                    count = count - 1;
                }
            }
        }
    }
    if (*reinterpret_cast<int *>(param_1 + 0x200) != 0) {
        fn_0064557f(reinterpret_cast<void *>(*reinterpret_cast<int *>(param_1 + 0x200)));
        *reinterpret_cast<int *>(param_1 + 0x200) = 0;
    }
    return 0;
}
