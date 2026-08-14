// ORIGINAL: 0x00634720 FILE
// working copy - scaffold materialised by --work
// name      sub_634720
// size      158 bytes
// spans     0x00634720-0x006347BE
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634720  at 0x00634720  (158 bytes)
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
// SIGNATURE CHANGED: `mov eax, ecx` at entry feeds the row-walking pointer
// with no matching stack slot for a1 - the receiver is `this` (thiscall),
// not the stdcall/int fallback.
extern "C" void *memcpy(void *, const void *, unsigned int);

class Mat3x3Rec { public:
    void addTransposed(float *dest, float *other);
};

void Mat3x3Rec::addTransposed(float *dest, float *other) {
    float tmp[9];
    tmp[0] = 1.0f; tmp[1] = 0.0f; tmp[2] = 0.0f;
    tmp[3] = 0.0f; tmp[4] = 1.0f; tmp[5] = 0.0f;
    tmp[6] = 0.0f; tmp[7] = 0.0f; tmp[8] = 1.0f;
    float *self = reinterpret_cast<float *>(this);
    int k = 0;
    int i = 3;
    do {
        int j = 3;
        do {
            tmp[k] = self[k] + other[k];
            k++;
            j--;
        } while (j != 0);
        i--;
    } while (i != 0);
    memcpy(dest, tmp, 9 * sizeof(float));
}
