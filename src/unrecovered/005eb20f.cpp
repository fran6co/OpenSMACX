// ORIGINAL: 0x005EB20F ?fill_scaling_table@Sprite@@QAEXXZ 0x005EB20F-0x005EB294 FILE
// size      133 bytes
// prototype void (__thiscall ?fill_scaling_table@Sprite@@QAEXXZ)(Sprite* this)
// callers   8   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?fill_scaling_table@Sprite@@QAEXXZ  at 0x005EB20F  (133 bytes)
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

class Sprite;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00696d18 = (int *)0x00696D18;
static int *const g_00696d1c = (int *)0x00696D1C;
static int *const g_009b3ad8 = (int *)0x009B3AD8;
static int *const g_009b4758 = (int *)0x009B4758;

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

    void fill_scaling_table();
};
// TRIED: ten statement-order permutations of the else-branch locals
//            (step/rem2/err/quotient, and remainder-as-named-var) all
//            reproduce the loop and division logic but land the
//            ebx/esi/edi push at the wrong point in the prologue - either
//            before the two initial loads (original pushes AFTER them) or
//            after the whole max/min block (original pushes BEFORE the
//            max/min compare). Swapping which of max/min is computed
//            first, and reordering the four local declarations, did not
//            change the outcome. The register allocator's push placement
//            is sensitive to overall register pressure in a way that did
//            not yield to source reordering alone within budget.
void Sprite::fill_scaling_table() {
    int max_dim = *g_00696d18;
    if (*g_00696d18 <= *g_00696d1c) {
        max_dim = *g_00696d1c;
    }
    int min_dim = *g_00696d1c;
    if (*g_00696d18 < *g_00696d1c) {
        min_dim = *g_00696d18;
    }

    int step;
    int rem2;
    int err;
    int quotient;
    if (min_dim < 2) {
        rem2 = 0;
        step = 0;
        err = 0;
        quotient = max_dim;
    } else {
        quotient = max_dim / min_dim;
        rem2 = (max_dim % min_dim) << 1;
        step = min_dim * 2;
        err = (rem2 >> 1) + min_dim * -2;
    }
    if ((quotient & 1) != 0) {
        err = err + max_dim;
    }
    int *p = g_009b3ad8;
    if (min_dim == 1) {
        for (int i = 800; i != 0; i = i - 1) {
            *p = quotient;
            p = p + 1;
        }
        return;
    }
    do {
        err = err + rem2;
        *p = quotient;
        if (0 < err) {
            err = err - step;
            *p = quotient + 1;
        }
        p = p + 1;
    } while (p < g_009b4758);
}
