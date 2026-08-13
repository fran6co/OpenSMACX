// ORIGINAL: 0x004C43F0 FILE
// working copy - scaffold materialised by --work
// name      ?pixel_to_tile@WorldWin@@QAEHHHPAH0@Z
// size      227 bytes
// spans     0x004C43F0-0x004C44D3
// prototype int (__thiscall ?pixel_to_tile@WorldWin@@QAEHHHPAH0@Z)(WorldWin* this, int, int, int*, int*)
// callers   3   call targets   0
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pixel_to_tile@WorldWin@@QAEHHHPAH0@Z  at 0x004C43F0  (227 bytes)
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

class WorldWin;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;

class WorldWin { public:
    int pixel_to_tile(int, int, int *, int *);
};
// RULED-OUT: MISMATCH at 0.905 mnemonic similarity (best of several
//            structural attempts). LEVERS THAT WORKED: (1) declaring
//            `x_num`/`y_num` (the a1/a2-derived locals) BEFORE
//            `width_div`/`height_div_neg` in the fast-path branch kept
//            the frame pointer (`push ebp; mov ebp,esp`) instead of it
//            being omitted - the omitted-frame-pointer form diverges at
//            instruction #3, right at the prologue. (2) writing the two
//            clip-bound checks as `a1 >= w + origin_x` / `a2 >= origin_y
//            - h` (matching original's actual comparison direction)
//            instead of the algebraically-equivalent `w + origin_x <=
//            a1` moved the divergence from a `jge`/`jle` polarity flip
//            at #31 to #37. RULED OUT beyond that: computing
//            `height_div_neg` immediately after reading `h` vs. deferring
//            it to just before its only use; reordering
//            `width_div`/`height_div_neg` before vs. after
//            `x_num`/`y_num` in the slow path. What remains at #37 is a
//            register/stack-slot allocation choice (original spills
//            `origin_y` into a1's now-dead stack slot and reloads it for
//            `y_num`; every spelling tried here keeps it in a register
//            instead) that did not yield to source reordering within
//            budget.
int WorldWin::pixel_to_tile(int a1, int a2, int * a3, int * a4) {
    char *self = reinterpret_cast<char *>(this);
    int origin_x = *reinterpret_cast<int *>(self + 0x239c4);
    int width_div;
    int height_div_neg;
    int x_num;
    int y_num;
    if (origin_x == 0 && *reinterpret_cast<int *>(self + 0x239c8) == 0) {
        x_num = a1;
        y_num = a2;
        width_div = *reinterpret_cast<int *>(self + 0x4c4);
        height_div_neg = -*reinterpret_cast<int *>(self + 0x4c8);
    } else {
        if (a1 < origin_x) {
            return 1;
        }
        int origin_y = *reinterpret_cast<int *>(self + 0x239c8);
        if (a2 < origin_y) {
            return 1;
        }
        int *vtable_ptr = *reinterpret_cast<int **>(self + 0x152c);
        int offset = *reinterpret_cast<int *>(reinterpret_cast<char *>(vtable_ptr) + 4);
        char *view = self + offset;
        int w = *reinterpret_cast<int *>(view + 0x19f0);
        if (a1 >= w + origin_x) {
            return 1;
        }
        int h = *reinterpret_cast<int *>(view + 0x19f4);
        if (a2 >= origin_y - h) {
            return 1;
        }
        width_div = w;
        height_div_neg = -h;
        x_num = a1 - origin_x;
        y_num = a2 - origin_y;
    }
    *a3 = (*g_00949870 * x_num) / width_div;
    int y_tile = (*g_00949874 * y_num) / height_div_neg;
    *a4 = y_tile;
    if ((*a3 & 1) == 0) {
        y_tile = y_tile & 0xfffffffe;
    } else {
        y_tile = y_tile | 1;
    }
    *a4 = y_tile;
    if (y_tile >= 0 && y_tile < *g_00949874 && *a3 >= 0 && *a3 < *g_00949870) {
        return 0;
    }
    return 1;
}
