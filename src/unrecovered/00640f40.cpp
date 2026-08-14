// ORIGINAL: 0x00640F40 FILE
// RULED-OUT: differently-named extern fn_00642a70(int,int,int,int) resolves fine
//            (not a link failure); mismatch is #1 mov-vs-push, whole-function
//            register-allocation order, not a logic difference
// working copy - scaffold materialised by --work
// name      sub_640f40
// size      179 bytes
// spans     0x00640F40-0x00640FF3
// prototype 
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00642A70

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_640f40  at 0x00640F40  (179 bytes)
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
extern "C" int __cdecl sub_642a70();
extern "C" int __cdecl fn_00642a70(int, int, int, int);

extern "C" int __cdecl sub_640f40(int param_1, int param_2, int param_3, int param_4) {
    char *base = reinterpret_cast<char *>(param_1);
    int bits = *reinterpret_cast<int *>(base + 0x16b4);
    if (bits < 14) {
        *reinterpret_cast<unsigned short *>(base + 0x16b0) =
            *reinterpret_cast<unsigned short *>(base + 0x16b0) |
            static_cast<unsigned short>(param_4 << (bits & 0x1f));
        *reinterpret_cast<int *>(base + 0x16b4) = bits + 3;
    } else {
        *reinterpret_cast<unsigned short *>(base + 0x16b0) =
            *reinterpret_cast<unsigned short *>(base + 0x16b0) |
            static_cast<unsigned short>(param_4 << (bits & 0x1f));
        *reinterpret_cast<unsigned char *>(*reinterpret_cast<int *>(base + 8) + *reinterpret_cast<int *>(base + 0x10)) =
            *reinterpret_cast<unsigned char *>(base + 0x16b0);
        int idx = *reinterpret_cast<int *>(base + 0x10) + 1;
        *reinterpret_cast<int *>(base + 0x10) = idx;
        *reinterpret_cast<unsigned char *>(*reinterpret_cast<int *>(base + 8) + idx) =
            *reinterpret_cast<unsigned char *>(base + 0x16b1);
        bits = *reinterpret_cast<int *>(base + 0x16b4);
        *reinterpret_cast<int *>(base + 0x10) = *reinterpret_cast<int *>(base + 0x10) + 1;
        *reinterpret_cast<int *>(base + 0x16b4) = bits - 0xd;
        *reinterpret_cast<unsigned short *>(base + 0x16b0) =
            static_cast<unsigned short>(param_4) >> ((0x10 - bits) & 0x1f);
    }
    *reinterpret_cast<unsigned int *>(base + 0x16a4) =
        ((*reinterpret_cast<unsigned int *>(base + 0x16a4) + 10) & 0xfffffff8) + 0x20 + param_3 * 8;
    return fn_00642a70(param_1, param_2, param_3, 1);
}
