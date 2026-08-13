// ORIGINAL: 0x006282E0 FILE
// working copy - scaffold materialised by --work
// name      sub_6282e0
// size      151 bytes
// spans     0x006282E0-0x00628377
// prototype 
// callers   2   call targets   0
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6282e0  at 0x006282E0  (151 bytes)
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
static int *const g_00670c18 = (int *)0x00670C18;
static int *const g_009c0b00 = (int *)0x009C0B00;
// PROPOSAL: signature -> extern "C" bool __cdecl sub_6282e0(float *a1,
// float a2, int a3). Evidence: `ret` with no epilogue clears only `al`
// on every branch (`mov al,1`/`xor al,al`), never `eax` - the callee-pop
// convention for a `bool` return under this compiler, not `int` (which
// would clear all of eax). Stack reads at [esp+4]/[esp+8]/[esp+0xc] with
// no prior push are three plain __cdecl arguments (float*, float, int),
// matching the Ghidra hint's arity.
//
// RULED-OUT (8 structural variants, none BYTE_EXACT - closest is
// 145/151 bytes, 8 edits, mnemonic_similarity 0.887, landed below):
// the divergence is entirely FPU SCHEDULING in the two `a1[0]`/`a1[1]`
// blocks, not logic. `a2 * K1 * K2` recomputed inline every use (4x);
// `delta` as one statement (`a2*K1*K2`) or negation cached in a second
// named local (`negDelta`); `a1[0]`/`a1[1]` read fresh at every use with
// no cache; a single compound `return A || B || ... ;` boolean-OR
// expression (far worse - 133 bytes, 0.52 similarity, computes `delta`
// unconditionally where the original is lazy). What got closest: `delta`
// split into two statements (`delta = a2*K1; delta *= K2;`) with
// `a1[0]`/`a1[1]` cached into named locals so the compiler keeps them
// FPU-resident (`fcompp` against a still-loaded register) instead of
// reloading from memory (`fcomp [mem]`) for the `>` checks - this alone
// reproduces the original's `fxch`/`fcompp` pattern for both `>` blocks
// byte-for-byte. What's left: the scheduler still hoists the cached
// `a1[0]` load to BEFORE `a2`'s own load (the original interleaves it
// mid-multiply, between `a2*K1` and `*K2`), and pays for keeping it
// resident with an extra `fstp st(0)` cleanup on the early-return path
// that the original doesn't have. Never found a source shape that gets
// the interleave position right AND avoids the extra cleanup at once.
extern "C" bool __cdecl sub_6282e0(float *a1, float a2, int a3) {
    if (a1[2] < *(float *)(a3 + 0x40)) {
        return true;
    }
    if (a1[2] > *(float *)(a3 + 0x44)) {
        return true;
    }
    float delta = a2 * *(float *)g_009c0b00;
    float x0 = a1[0];
    delta *= *(float *)g_00670c18;
    if (x0 < -delta) {
        return true;
    }
    if (x0 > (float)*(int *)(a3 + 0x58) + delta) {
        return true;
    }
    if (a1[1] < -delta) {
        return true;
    }
    if (a1[1] > (float)*(int *)(a3 + 0x5c) + delta) {
        return true;
    }
    return false;
}
