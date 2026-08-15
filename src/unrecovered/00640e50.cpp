// ORIGINAL: 0x00640E50 BYTE_EXACT FILE
// name      _init_block
// CORRECTED from sub_640e50
//   zlib 1.0.2 trees.c, 102 bytes, byte-exact from upstream
// size      102 bytes
// spans     0x00640E50-0x00640EB6
// prototype 
// callers   2   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_640e50  at 0x00640E50  (102 bytes)
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
extern "C" void __cdecl sub_640e50(int param_1) {
    short *field_ptr = (short *)(param_1 + 0x88);
    int count = 0x11e;
    do {
        *field_ptr = 0;
        field_ptr += 2;
        count = count - 1;
    } while (count != 0);

    field_ptr = (short *)(param_1 + 0x97c);
    count = 0x1e;
    do {
        *field_ptr = 0;
        field_ptr += 2;
        count = count - 1;
    } while (count != 0);

    field_ptr = (short *)(param_1 + 0xa70);
    count = 0x13;
    do {
        *field_ptr = 0;
        field_ptr += 2;
        count = count - 1;
    } while (count != 0);

    *(int *)(param_1 + 0x16a0) = 0;
    *(int *)(param_1 + 0x169c) = 0;
    *(int *)(param_1 + 0x16a8) = 0;
    *(int *)(param_1 + 0x1694) = 0;
    *(short *)(param_1 + 0x488) = 1;
}
