// ORIGINAL: 0x005EAC6F ?get_clipped_x_table_shrink@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z 0x005EAC6F-0x005EACFB FILE
// size      140 bytes
// prototype int (__thiscall ?get_clipped_x_table_shrink@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z)(Spot* this, RECT*, RECT*, int*, int*, int*, int*)
// callers   8   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_clipped_x_table_shrink@Sprite@@QAEHPAURECT@@PAURECT@@PAHPAHPAHPAH@Z  at 0x005EAC6F  (140 bytes)
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

    int get_clipped_x_table_shrink(RECT *, RECT *, int *, int *, int *, int *);
};
// TRIED: reaches instruction 37/56 exactly (138 of 140 bytes) then
// diverges by one extra `mov`. The original keeps EDX as the *a4 accumulator
// pointer through the first loop and loads table1's marching pointer fresh
// into ECX; every source form tried here gets those two roles swapped
// (accumulator in ECX, table pointer in EDX), which costs one spurious
// reload. Tried and worse/no-better: `base = span` moved before the loop
// (regresses to #25), storing *a4 before *a5 (regresses to #21), an
// explicit `int *p4 = a4` cache, a plain temp read before the `+=`, an
// if/else that duplicates the `*a6 = 0` store (bloats to 159 bytes), and a
// fully pointer-marching rewrite of loop 1 (regresses to #7). Whatever
// distinguishes the original's register choice here is not visible in any
// of these source-level reorderings.
extern int g_x_shift_table[];
extern int g_x_clip_table[];

int Sprite::get_clipped_x_table_shrink(RECT * a1, RECT * a2, int * a3, int * a4, int * a5, int * a6) {
    int span = a2->left - a1->left;
    int width = a2->right - a2->left;
    if (width == 0) {
        return 0;
    }
    int base = 0;
    *a3 = span;
    *a5 = width;
    *a4 = 0;
    if (span > 0) {
        for (int i = 0; i < span; i++) {
            *a4 += g_x_shift_table[i];
        }
        base = span;
    }
    *a6 = 0;
    if (width > 0) {
        for (int i = 0; i < width; i++) {
            int v = g_x_shift_table[base + i];
            g_x_clip_table[i] = v;
            *a6 += v;
        }
    }
    return (int)g_x_clip_table;
}
