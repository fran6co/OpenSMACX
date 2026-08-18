// ORIGINAL: 0x005D3F50 sub_5d3f50 0x005D3F50-0x005D4014 FILE
// RULED-OUT: signature cdecl() -> fastcall(int *param_1); evidence: ecx read directly at entry (dereferenced once for an object pointer, re-read every loop iteration), no stack access, plain `ret` (no callee-pop). Dropped Ghidra's explicit `& 0x1f` shift masks per the known lever (extra `and`); still MISMATCH at #4, likely register-allocation/prologue shape for the three precomputed shift amounts.
// working copy - scaffold materialised by --work
// size      196 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d3f50  at 0x005D3F50  (196 bytes)
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
extern "C" void __fastcall sub_5d3f50(int *param_1) {
    int obj0 = *param_1;
    unsigned char shiftB = (unsigned char)(8 - *(int *)(obj0 + 0x30));
    unsigned char shiftC = (unsigned char)(8 - *(int *)(obj0 + 0x2c));
    unsigned char shiftA = (unsigned char)(8 - *(int *)(obj0 + 0x34));

    int i = 0;
    int outOff = 0x3b0;
    do {
        int obj = *param_1;
        unsigned int term1 = *(unsigned char *)(obj + 0xb1 + i);
        term1 = (term1 >> shiftB) << *(int *)(obj + 0x3c);

        unsigned int term2 = *(unsigned char *)(obj + 0xb0 + i);
        term2 = (term2 >> shiftC) << *(int *)(obj + 0x38);

        unsigned int result = term1 | term2;

        unsigned int term3 = *(unsigned char *)(obj + 0xb2 + i);
        term3 = (term3 >> shiftA) << *(int *)(obj + 0x40);

        result |= term3;

        if (*(unsigned char *)(obj + 4) & 0x40) {
            result |= result << 0x10;
        }
        *(unsigned int *)(obj + outOff) = result;
        outOff += 4;
        i += 3;
    } while (outOff < 0x7b0);
}
