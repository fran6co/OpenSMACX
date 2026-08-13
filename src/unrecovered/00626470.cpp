// ORIGINAL: 0x00626470 FILE
// working copy - scaffold materialised by --work
// name      sub_626470
// size      208 bytes
// spans     0x00626470-0x00626540
// prototype 
// callers   3   call targets   0
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_626470  at 0x00626470  (208 bytes)
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
static int *const g_009bc010 = (int *)0x009BC010;
static int *const g_009bc018 = (int *)0x009BC018;
static int *const g_009bc038 = (int *)0x009BC038;

// SIGNATURE CHANGE: two stack args are read ([esp+0x14], [esp+0x18], no ecx
// use at all) - not the zero-arg contract. param_1 is a 9-int output record
// (fields written at +4,+8,+0xc,+0x10,+0x14,+0x18,+0x1c,+0x20); param_2 is
// a start index into a circular table. See PROPOSALS in the agent report.
extern "C" int __cdecl sub_626470(int *param_1, int param_2) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);
    //
    // RULED-OUT: 200/208 bytes (edit distance 4, first divergence in the
    // prologue). Original pushes ebx/ebp/esi/edi (4 registers, full
    // register pressure) before the first check; every source form tried
    // here - named temps for each table read, a `volatile` temp to block
    // memory-operand folding, an `extern int32_t[]` single-indirection
    // table model, caching `param_1[0]` vs re-reading it, hoisting
    // `*g_009bc038` vs re-reading it - only ever gets 3 registers pushed,
    // never 4, because /O2 folds one table read into the `sub`'s memory
    // operand no matter how the source asks for it. The double-indirection
    // model (`*g_009bc018` re-read fresh at every table access, matching
    // the original's repeated `mov reg,[0x9bc018]` around each store to
    // *param_1 - an aliasing-conservative reload the compiler evidently
    // makes on its own) is closest and is what is landed.
    if (param_2 == *g_009bc010) {
        return 0;
    }
    int value0 = param_1[0];
    int index;
    do {
        index = value0 + param_2;
        if (index > *g_009bc038 - 1) {
            index = 0;
        } else if (index < 0) {
            index = *g_009bc038 - 1;
        }
        int diff = *(int *)(*g_009bc018 + 4 + index * 8)
                 - *(int *)(*g_009bc018 + 4 + param_2 * 8);
        param_1[1] = diff;
        if (diff != 0) {
            if (diff < 0) {
                return 0;
            }
            param_1[2] = index;
            param_1[3] = *(int *)(*g_009bc018 + param_2 * 8);
            int delta = *(int *)(*g_009bc018 + index * 8)
                      - *(int *)(*g_009bc018 + param_2 * 8);
            int quotient;
            if (delta < 0) {
                param_1[5] = -1;
                delta = -delta;
                param_1[6] = 1 - param_1[1];
                quotient = -(delta / param_1[1]);
            } else {
                param_1[5] = 1;
                quotient = delta / param_1[1];
                param_1[6] = 0;
            }
            param_1[4] = quotient;
            param_1[7] = delta % param_1[1];
            param_1[8] = param_1[1];
            return 1;
        }
        param_2 = index;
    } while (index != *g_009bc010);
    return 0;
}
