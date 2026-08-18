// ORIGINAL: 0x005D439F sub_5d439f 0x005D439F-0x005D4448 FILE
// working copy - scaffold materialised by --work
// size      169 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d439f  at 0x005D439F  (169 bytes)
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
// IDA guessed int __cdecl(int,int,int,int); the tail never sets eax, so the
// real return type is void, and evidence (only [esp+N] reads, no ecx use)
// backs the __cdecl free-function shape with 4 stack ints.
extern "C" void __cdecl sub_5d439f(unsigned int *a1, int a2, int a3, int a4) {
    unsigned int bitpos;
    unsigned int mask;
    unsigned char width;
    unsigned char shift;
    unsigned int word;

    bitpos = 0;
    width = (unsigned char)a4;
    mask = (1 << (width & 0x1f)) - 1;
    while (a3 > 3) {
        shift = (unsigned char)bitpos & 7;
        word = *(unsigned int *)((bitpos >> 3) + a2);
        word = (word >> shift) | (word << (0x20 - shift));
        *a1 = word & mask;
        a1[1] = ((word >> (width & 0x1f)) | (word << (0x20 - (width & 0x1f)))) & mask;

        shift = (unsigned char)(bitpos + width * 2) & 7;
        word = *(unsigned int *)(((bitpos + a4 * 2) >> 3) + a2);
        word = (word >> shift) | (word << (0x20 - shift));
        a1[2] = word & mask;
        a1[3] = ((word >> (width & 0x1f)) | (word << (0x20 - (width & 0x1f)))) & mask;

        a1 += 4;
        bitpos += a4 * 4;
        a3 -= 4;
    }
    if (a3 - 4 > -4) {
        do {
            a3--;
            shift = (unsigned char)bitpos & 7;
            word = *(unsigned int *)((bitpos >> 3) + a2);
            *a1 = ((word >> shift) | (word << (0x20 - shift))) & mask;
            bitpos += a4;
            a1++;
        } while (a3 > 0);
    }
}
