// ORIGINAL: 0x006347F0 FILE
// working copy - scaffold materialised by --work
// name      sub_6347f0
// size      204 bytes
// spans     0x006347F0-0x006348BC
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6347f0  at 0x006347F0  (204 bytes)
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
// Signature changed from the fallback `extern "C" int __stdcall(int)`: the
// disassembly reads ecx as a plain pointer (no stack-passed first arg) and
// a second pointer at the first stack slot after the prologue, with `ret 4`
// popping that one slot - a __thiscall member taking one pointer argument,
// not an __stdcall free function. No mangled name exists, so a helper class
// expresses the thiscall receiver.
class Obj6347f0 {
public:
    void sub_6347f0(float *a2);
};

void Obj6347f0::sub_6347f0(float *a2) {
    float *self = reinterpret_cast<float *>(this);
    float tmp[9] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float *row = self;
    float *dst = tmp;
    for (int i = 3; i != 0; --i) {
        float *col = a2;
        for (int j = 3; j != 0; --j) {
            float sum = 0.0f;
            float *a = row;
            float *b = col;
            for (int k = 3; k != 0; --k) {
                sum = (*b) * (*a) + sum;
                ++a;
                b += 3;
            }
            *dst = sum;
            ++dst;
            ++col;
        }
        row += 3;
    }
    for (int idx = 0; idx < 9; ++idx) {
        self[idx] = tmp[idx];
    }
}
