// ORIGINAL: 0x005CE640 FILE
// working copy - scaffold materialised by --work
// name      sub_5ce640
// size      163 bytes
// spans     0x005CE640-0x005CE6E3
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ce640  at 0x005CE640  (163 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00694df4 = (int *)0x00694DF4;
// Evidence: only [ebp+N] reads, no ecx use at all, and a plain `ret` (no
// operand) - a free __cdecl function with 6 stack int parameters, not the
// zero-arg fallback head.
extern "C" void __cdecl sub_5ce640(int a1, int a2, int a3, int a4, int a5,
                                    int a6) {
    unsigned int count;
    unsigned char *src;
    short *dst;
    char *stepBase;
    int predicted;
    unsigned int nibble;
    unsigned char byteVal;

    count = 0;
    count += a1;
    a4 += a1;
    stepBase = *(char **)a6;
    predicted = (short)*(int *)a5;
    src = (unsigned char *)a2;
    dst = (short *)a3;

    do {
        byteVal = *src;
        if ((count & 1) == 0) {
            nibble = (unsigned char)(byteVal >> 4);
        } else {
            nibble = byteVal & 0xF;
            src++;
        }
        predicted += *(int *)(stepBase + nibble * 4 + 0x6937b4);
        if (predicted > 0x7FFF) {
            predicted = 0x7FFF;
        } else if (predicted < (int)0xFFFF8000) {
            predicted = (int)0xFFFF8000;
        }
        *dst = (short)predicted;
        dst++;
        count++;
        nibble &= 7;
        stepBase += g_00694df4[nibble];
        if ((int)stepBase > 0x1600) {
            stepBase = (char *)0x1600;
        } else if ((int)stepBase < 0) {
            stepBase = (char *)0;
        }
    } while (count < (unsigned int)a4);

    *(int *)a5 = predicted;
    *(int *)a6 = (int)stepBase;
}
