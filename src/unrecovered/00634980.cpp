// ORIGINAL: 0x00634980 FILE
// RULED-OUT: `extern "C" int __stdcall sub_634980(int a1, int a2)` per the
//   given def head - disasm reads *ecx directly as a float* with no matching
//   stack slot (a 3x3 matrix "this"), so the receiver is a __thiscall
//   member, not a 2-arg __stdcall free function; changed to a `Mat3::multiply`
//   member. `dst`/`a` tracked as independent incrementing pointers gives a
//   MISMATCH at the prologue (#4, 87% similarity); an `int off = dst - a`
//   delta recomputed each outer iteration (matching the original's spill of
//   that delta to `[esp+0x14]` and its `add esi, ebx` re-derivation) gets to
//   89.5% similarity, 7 edits, all register-allocation choices (which
//   callee-saved reg holds which pointer) rather than control-flow shape.
// working copy - scaffold materialised by --work
// name      sub_634980
// size      208 bytes
// spans     0x00634980-0x00634A50
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634980  at 0x00634980  (208 bytes)
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
// A 3x3 row-major float matrix. `this` is the left operand (matches ecx
// dereferenced directly as a float base, no field offsets), `other` is the
// right operand, `out` receives the product - see RULED-OUT above for the
// signature evidence.
class Mat3 {
public:
    void multiply(float *out, float *other);
};

void Mat3::multiply(float *out, float *other) {
    float *a = reinterpret_cast<float *>(this);
    float local[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    int off = reinterpret_cast<int>(local) - reinterpret_cast<int>(a);
    int row = 3;
    do {
        float *dst = reinterpret_cast<float *>(off + reinterpret_cast<int>(a));
        float *b = other;
        int col = 3;
        do {
            *dst = 0.0f;
            float sum = *dst;
            const float *pa = a;
            const float *pb = b;
            int k = 3;
            do {
                sum = *pa * *pb + sum;
                pa = pa + 1;
                pb = pb + 3;
                k = k - 1;
            } while (k != 0);
            *dst = sum;
            dst = dst + 1;
            b = other + (3 - col) + 1;
            col = col - 1;
        } while (col != 0);
        a += 3;
        --row;
    } while (row != 0);
    for (int i = 0; i < 9; ++i) {
        out[i] = local[i];
    }
}
