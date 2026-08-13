// ORIGINAL: 0x006428C0 FILE
// working copy - scaffold materialised by --work
// name      sub_6428c0
// size      116 bytes
// spans     0x006428C0-0x00642934
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6428c0  at 0x006428C0  (116 bytes)
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
extern "C" void __cdecl sub_6428c0(int a1) {
    unsigned short *p;
    unsigned int sum1 = 0;
    unsigned int sum2 = 0;
    int n;

    p = reinterpret_cast<unsigned short *>(a1 + 0x88);
    n = 7;
    do {
        sum1 += *p;
        p += 2;
        --n;
    } while (n != 0);

    p = reinterpret_cast<unsigned short *>(a1 + 0xA4);
    n = 0x79;
    int total = 7 + 0x79;
    do {
        sum2 += *p;
        p += 2;
        --n;
    } while (n != 0);

    if (total < 0x100) {
        p = reinterpret_cast<unsigned short *>(a1 + 0x88 + total * 4);
        n = 0x100 - total;
        do {
            sum1 += *p;
            p += 2;
            --n;
        } while (n != 0);
    }

    *reinterpret_cast<unsigned char *>(a1 + 0x18) = (sum1 <= (sum2 >> 2));
}
