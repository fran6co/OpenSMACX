// ORIGINAL: 0x00640EC0 FILE
// working copy - scaffold materialised by --work
// name      sub_640ec0
// size      115 bytes
// spans     0x00640EC0-0x00640F33
// prototype 
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00642940

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_640ec0  at 0x00640EC0  (115 bytes)
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
extern "C" int __cdecl sub_642940();
extern "C" int __cdecl sub_642940_impl(int, int);

extern "C" int __cdecl sub_640ec0(unsigned short *param_1, int param_2, char *param_3) {
    short code_table[16];
    short accum = 0;
    int i;
    for (i = 1; i <= 15; ++i) {
        accum = (*reinterpret_cast<short *>(param_3 + (i - 1) * 2) + accum) * 2;
        code_table[i] = accum;
    }
    if (param_2 >= 0) {
        int count = param_2 + 1;
        unsigned short *p = param_1;
        do {
            unsigned int len = p[1];
            if (len != 0) {
                short code = code_table[len];
                code_table[len] = code + 1;
                *p = static_cast<unsigned short>(sub_642940_impl(code, len));
            }
            p += 2;
            count -= 1;
        } while (count != 0);
    }
    return 0;
}
