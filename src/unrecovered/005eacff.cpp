// ORIGINAL: 0x005EACFF FILE
// name      ?get_clipped_y_table_shrink@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z
// size      142 bytes
// spans     0x005EACFF-0x005EAD8D
// prototype int (__thiscall ?get_clipped_y_table_shrink@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z)(Sprite* this, RECT*, RECT*, int*, int*, int*, int*)
// callers   8   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_clipped_y_table_shrink@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z  at 0x005EACFF  (142 bytes)
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

    int get_clipped_y_table_shrink(RECT *, RECT *, int *, int *, int *, int *);
};
// RULED-OUT: BYTE_EXACT not reached in 10 candidates. The whole function
// matches except ONE instruction: original stores the `*a4 = 0;` guard as
// an immediate (`c7 02 00000000`, 6 bytes) even though a callee-saved
// register (ebx, holding the `index` local seen below) provably holds 0 at
// that exact point; every source form tried here that keeps `int index = 0;`
// live before the `if (dy > 0)` block (needed so `index`/ebx is pushed and
// zeroed EARLY, matching the original's instruction order up to this point)
// makes VC6 reuse that register for the store instead (`89 1a`, 2 bytes) -
// a 4-byte shrink that is the ONLY diff (confirmed via raw object bytes).
// Restructuring `index` as an if/else (no early unconditional `= 0`), a
// ternary computed at the use site, or reusing the loop pointer instead of
// a separate `index` all avoid the register-reuse but regress to an EARLIER
// divergence (`ebx` no longer pushed/zeroed early, losing the `test` for
// the len-scoped guard). `volatile int *a4` store also regressed. Tried:
// index declared before vs after `*a3/*a5/*a4`; index set inside vs after
// the accumulation loop; if/else assignment for `index` and for the second
// loop's start pointer; reusing the advanced table pointer directly;
// ternary `dy > 0 ? dy : 0` computed at the second loop only.
int Sprite::get_clipped_y_table_shrink(RECT * a1, RECT * a2, int * a3, int * a4, int * a5, int * a6) {
    extern int table_lo[];
    extern int table_hi[];

    int dy = a2->top - a1->top;
    int height = a2->bottom - a2->top;
    if (height == 0) {
        return 0;
    }
    int index = 0;
    *a3 = dy;
    *a5 = height;
    *a4 = 0;
    if (dy > 0) {
        index = dy;
        int *p = table_lo;
        int n = dy;
        do {
            *a4 += *p;
            p++;
            n--;
        } while (n != 0);
    }
    *a6 = 0;
    if (height > 0) {
        int *src = table_lo + index;
        int *dst = table_hi;
        int n = height;
        do {
            int v = *src;
            src++;
            *dst = v;
            dst++;
            *a6 += v;
            n--;
        } while (n != 0);
    }
    return reinterpret_cast<int>(table_hi);
}
