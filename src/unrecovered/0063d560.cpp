// ORIGINAL: 0x0063D560 FILE
// working copy - scaffold materialised by --work
// name      sub_63d560
// size      147 bytes
// spans     0x0063D560-0x0063D5F3
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63d560  at 0x0063D560  (147 bytes)
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
struct Rec698cf0 { unsigned short f0; unsigned short f2; unsigned short f4; unsigned short f6; unsigned short pad1; unsigned short pad2; };
extern Rec698cf0 g_table_698cf0[];
extern "C" void *memset(void *, int, unsigned int);

extern "C" void __cdecl sub_63d560(int param1) {
    char *p = reinterpret_cast<char *>(param1);

    *reinterpret_cast<int *>(p + 0x30) = *reinterpret_cast<int *>(p + 0x20) << 1;

    char *buf = *reinterpret_cast<char **>(p + 0x38);
    int count = *reinterpret_cast<int *>(p + 0x40);
    int len = count * 2 - 2;

    *reinterpret_cast<short *>(buf + len) = 0;

    char *fillBuf = *reinterpret_cast<char **>(p + 0x38);
    memset(fillBuf, 0, len);

    int idx = *reinterpret_cast<int *>(p + 0x78);
    Rec698cf0 *rec = &g_table_698cf0[idx];

    *reinterpret_cast<unsigned int *>(p + 0x74) = rec->f2;
    *reinterpret_cast<unsigned int *>(p + 0x80) = rec->f0;
    *reinterpret_cast<unsigned int *>(p + 0x84) = rec->f4;
    unsigned int f6val = rec->f6;

    *reinterpret_cast<int *>(p + 0x60) = 0;
    *reinterpret_cast<int *>(p + 0x50) = 0;
    *reinterpret_cast<int *>(p + 0x68) = 0;
    *reinterpret_cast<int *>(p + 0x5c) = 0;
    *reinterpret_cast<int *>(p + 0x3c) = 0;
    *reinterpret_cast<unsigned int *>(p + 0x70) = f6val;
    *reinterpret_cast<int *>(p + 0x6c) = 2;
    *reinterpret_cast<int *>(p + 0x54) = 2;
}
