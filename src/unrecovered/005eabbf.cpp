// ORIGINAL: 0x005EABBF ?get_clipped_y_table_expand@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z 0x005EABBF-0x005EAC69 FILE
// size      170 bytes
// prototype int (__thiscall ?get_clipped_y_table_expand@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z)(Spot* this, RECT*, RECT*, int*, int*, int*, int*)
// callers   8   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_clipped_y_table_expand@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z  at 0x005EABBF  (170 bytes)
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
static int *const g_009b4760 = (int *)0x009B4760;
static int *const g_009b4764 = (int *)0x009B4764;

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

    int get_clipped_y_table_expand(RECT *, RECT *, int *, int *, int *, int *);
};
// TRIED: MNEMONIC_ONLY (66/66 mnemonics match; not BYTE_EXACT). In the
//            negative-remainder branch, original loads `a3`'s pointer
//            EARLY (right after the `jge`, before `neg`/`sub`/`dec`) but
//            still stores through it SECOND, after `*g_009b4760 = ...`.
//            Every statement order that keeps the g_009b4760-then-a3
//            store order (matching original) makes the compiler load a3
//            LATE instead (a 5-instruction MISMATCH); writing `*a3 = i;`
//            as the first store makes it load a3 early as a side effect,
//            but then stores through it FIRST (wrong order either way).
//            Explicitly hoisting `int *a3_ptr = a3;` before the branch
//            body did not change the scheduling in either direction.
//            Landed the MNEMONIC_ONLY form (store-order swapped) since it
//            is a strictly higher tier than the alternative.
int Sprite::get_clipped_y_table_expand(RECT * a1, RECT * a2, int * a3, int * a4, int * a5, int * a6) {
    int y_delta = a2->top - a1->top;
    int height = a2->bottom - a2->top;
    if (height == 0) {
        return 0;
    }
    *a6 = height;
    *a4 = y_delta;
    int i = 0;
    if (y_delta > 0) {
        int *table = g_009b3ad8;
        do {
            y_delta -= *table;
            table++;
            i++;
        } while (y_delta > 0);
    }
    int *p = g_009b3ad8 + i;
    int *dest;
    int rows_written;
    if (y_delta < 0) {
        y_delta = -y_delta;
        height -= y_delta;
        i--;
        *a3 = i;
        *g_009b4760 = y_delta;
        dest = g_009b4764;
        rows_written = 1;
    } else {
        dest = g_009b4760;
        *a3 = i;
        rows_written = 0;
    }
    while (height > 0) {
        int v = *p;
        p++;
        *dest = v;
        dest++;
        height -= v;
        rows_written++;
    }
    if (height < 0) {
        dest--;
        *dest = *dest + height;
    }
    *a5 = rows_written;
    return (int)g_009b4760;
}
