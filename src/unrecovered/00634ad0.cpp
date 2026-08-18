// ORIGINAL: 0x00634AD0 sub_634ad0 0x00634AD0-0x00634B67 FILE
// RULED-OUT: extern "C" __stdcall(int,int) contract head - disasm reads ecx as an implicit receiver (thiscall) into a 3x3 float matrix, not a stack int; rewrote as Mat3x3::scale_into(float*,float). MISMATCH #1 push/lea, prologue layout only, structure otherwise matches Ghidra's nested-loop scale+copy.
// working copy - scaffold materialised by --work
// size      151 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634ad0  at 0x00634AD0  (151 bytes)
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
class Mat3x3 { public:
    float m_[9];
    void scale_into(float *dest, float scale);
};

void Mat3x3::scale_into(float *dest, float scale) {
    float tmp[9] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float *pf1 = tmp;
    int base = (int)this - (int)tmp;
    int i4 = 3;
    do {
        int i3 = 3;
        float *pf2 = pf1;
        do {
            pf1 = pf2 + 1;
            i3 = i3 - 1;
            *pf2 = scale * *(float *)(base + (int)pf2);
            pf2 = pf1;
        } while (i3 != 0);
        i4 = i4 - 1;
    } while (i4 != 0);
    pf1 = tmp;
    for (i4 = 9; i4 != 0; i4 = i4 - 1) {
        *dest = *pf1;
        pf1 = pf1 + 1;
        dest = dest + 1;
    }
}
