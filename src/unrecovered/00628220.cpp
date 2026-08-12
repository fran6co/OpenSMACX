// ORIGINAL: 0x00628220 FILE
// name      sub_628220
// size      107 bytes
// spans     0x00628220-0x0062828B
// prototype 
// callers   10   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: no-locals src[i]*src[i] (every order/assoc/operand-order/1-vs-2-statement reciprocal spelling, 5760 units, flat 69 B); locals x,y,z with the tail reading src[] (69-76 B, no spill); tail reading the locals (85-95 B, sub esp,8 - one slot too many); const src, /Oa, __inline helper, aliasing store before the sum, dead stores to locals, sum-in-a-variable - none spill y,z; /Op and /O1 both lose the fsqrt intrinsic to a call
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_628220  at 0x00628220  (107 bytes)
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
static int *const g_00670c08 = (int *)0x00670C08;
// The disassembly reads [esp+8] and [esp+0xc] as pointers and writes nothing
// to eax, so the nullary `int` head the emitter guessed cannot express this
// body; see PROPOSALS in the report.
extern "C" double __cdecl sqrt(double);

extern "C" void __cdecl sub_628220(float *src, float *dst) {
    float x = src[0];
    float y = src[1];
    float z = src[2];
    float f = (float)sqrt(x * x + z * z + y * y);
    f = *(float *)g_00670c08 / f;
    dst[0] = src[0] * f;
    dst[1] = f * y;
    dst[2] = f * z;
}
