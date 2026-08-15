// ORIGINAL: 0x005EAB0F FILE
// name      ?get_clipped_x_table_expand@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z
// size      168 bytes
// spans     0x005EAB0F-0x005EABB7
// prototype int (__thiscall ?get_clipped_x_table_expand@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z)(Spot* this, RECT*, RECT*, int*, int*, int*, int*)
// callers   8   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_clipped_x_table_expand@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z  at 0x005EAB0F  (168 bytes)
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

struct RECT;
class Sprite;

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b3ad8 = (int *)0x009B3AD8;
static int *const g_009b53ec = (int *)0x009B53EC;
static int *const g_009b53f0 = (int *)0x009B53F0;

class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;

    int get_clipped_x_table_expand(RECT *, RECT *, int *, int *, int *, int *);
};
int Sprite::get_clipped_x_table_expand(RECT * a1, RECT * a2, int * a3, int * a4, int * a5, int * a6) {
    // Measured 161/168 bytes; first divergence at instruction #32 of 66,
    // 'mov' vs 'neg' - a pure reordering, same opcodes and operands, so
    // operand_similarity 0.589 beats every other spelling tried.
    //
    // RULED-OUT: ~18 source-form variants across three independent axes,
    // none reaching BYTE_EXACT:
    //  - branch polarity on the excess<0 test: `if (excess>=0){pos}else{neg}`
    //    compiles pos-block-first with `jl`/`jle`; only `if (excess<0)
    //    {neg}else{pos}` reproduces the original's `jge`-to-positive-label
    //    layout (LEVER, used below).
    //  - the negative-branch width adjustment: original computes `neg eax`
    //    ONCE and reuses that register for BOTH the `DAT_9b53ec = -excess`
    //    store AND `width -= -excess` (via `sub`, not `add`). A named
    //    `int negExcess = -excess;` temp reproduces that reuse (used below)
    //    but then the a3-pointer `mov` that precedes it in the original
    //    schedules AFTER the neg/sub/dec instead of before - tried moving
    //    the a3-pointer read first as its own statement, tried it as the
    //    block's very first line, tried `store = temp = -excess` comma-form:
    //    none hoist it. Writing `-excess` inline twice (no temp) DOES keep
    //    the a3-pointer mov first, but the compiler then uses `add` for the
    //    width update instead of reusing a negation - so the two divergences
    //    (mov-order vs neg/add) trade off against each other and no
    //    variant tried gets both.
    //  - the positive-branch `count = 0` (`xor edi,edi`): reordering
    //    `dst = ...; *a3 = ...; count = 0;` inside that branch closed the
    //    `xor` mismatch but reopened a NEW, earlier divergence at
    //    instruction #16 (the `*a6`/`*a4` stores + `shift = 0` init),
    //    confirming this is whole-function register scheduling, not a
    //    per-block reordering a source change can target in isolation.
    // Landing the best-scoring spelling (operand_similarity 0.589,
    // mnemonic_similarity 0.930, 7-instruction edit distance) rather than
    // the fewer-edits/worse-operands alternative (0.492 similarity, 6-edit).

    // g_009b3ad8 is indexed via `lea reg,[count*4+addr]` below, which /O2
    // folds away through the scaffold's const-pointer spelling (the address
    // does work here, not just a load/store) - so it gets its own `extern`
    // binding instead of the file's g_009b3ad8 const pointer. Named to match
    // the sibling 0x005EAC6F (get_clipped_x_table_shrink), which reads the
    // same table through this role.
    extern int g_x_shift_table[];

    int excess = a2->left - a1->left;
    int width = a2->right - a2->left;
    if (width == 0) {
        return 0;
    }

    *a6 = width;
    *a4 = excess;

    int shift = 0;
    if (excess > 0) {
        int *src = g_x_shift_table;
        do {
            excess -= *src;
            src++;
            shift++;
        } while (excess > 0);
    }
    int *cursor = g_x_shift_table + shift;

    int *dst;
    int count;
    if (excess < 0) {
        int negExcess = -excess;
        width -= negExcess;
        shift--;
        g_009b53ec[0] = negExcess;
        *a3 = shift;
        dst = g_009b53f0;
        count = 1;
    } else {
        *a3 = shift;
        dst = g_009b53ec;
        count = 0;
    }

    while (width > 0) {
        int step = *cursor;
        cursor++;
        *dst = step;
        dst++;
        width -= step;
        count++;
    }
    if (width < 0) {
        dst[-1] += width;
    }

    *a5 = count;
    return (int)g_009b53ec;
}
