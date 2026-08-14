// ORIGINAL: 0x00634520 FILE
// RULED-OUT: extern "C" int __stdcall sub_634520(int) per scaffold contract
//            (no this, wrong callee: disasm reads ecx=this Vec3 and a stack
//            dest ptr, ret 4). Rewritten as Vec3F::normalize(float*)
//            thiscall; differently-named sqrt_impl(double) sidesteps the
//            scaffold's nullary sqrt(). MISMATCH: FPU stack scheduling and
//            extra stack zero-stores not reproduced (153 vs 189 bytes)
// working copy - scaffold materialised by --work
// name      sub_634520
// size      189 bytes
// spans     0x00634520-0x006345DD
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00646544

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634520  at 0x00634520  (189 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl sqrt();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066aa34 = (int *)0x0066AA34;
extern "C" double __cdecl sqrt_impl(double);

class Vec3F { public:
    float x_, y_, z_;
    void normalize(float *dest);
};

void Vec3F::normalize(float *dest) {
    double lenSq = (double)(z_ * z_ + y_ * y_ + x_ * x_);
    double len = sqrt_impl(lenSq);
    float constVal = *reinterpret_cast<float *>(g_0066aa34);
    if (len == constVal) {
        *dest = x_;
        dest[1] = y_;
        dest[2] = z_;
        return;
    }
    double inv = constVal / len;
    float y = y_;
    float z = z_;
    *dest = (float)(inv * x_);
    dest[1] = (float)(inv * y);
    dest[2] = (float)(inv * z);
}
