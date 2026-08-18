// ORIGINAL: 0x005B5E10 sub_5b5e10 0x005B5E10-0x005B5EA0 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      144 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5b5e10  at 0x005B5E10  (144 bytes)
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
extern unsigned char g_95282c[];

extern "C" void __cdecl sub_5b5e10(int a1) {
    int offset = a1 * 0x34;
    unsigned int *flags = reinterpret_cast<unsigned int *>(g_95282c + offset);
    unsigned char *byteD = g_95282c + offset + 0xD;
    unsigned char *byteE = g_95282c + offset + 0xE;
    unsigned char *byteF = g_95282c + offset + 0xF;
    if ((*flags & 0x2000200) == 0x2000200 && *byteF < *byteE) {
        *flags &= 0xfdfffdff;
    }
    if ((*flags & 0x1000200) == 0x1000200) {
        *flags &= 0xfefffdff;
    }
    *flags &= 0xffffbfff;
    if (*byteD == 0x18) {
        *byteD = 0;
    }
}
