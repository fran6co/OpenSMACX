// ORIGINAL: 0x004C44E0 FILE
// name      ?tile_to_pixel@WorldWin@@QAEHHHPAH0@Z
// size      242 bytes
// spans     0x004C44E0-0x004C45D2
// prototype int (__thiscall ?tile_to_pixel@WorldWin@@QAEHHHPAH0@Z)(WorldWin* this, int, int, int*, int*)
// callers   3   call targets   0
// kind      
// flags     
// calls     (none)
// RULED-OUT: nested `if`/`else if` clamp plus a single `||`-chained bounds
//            check (`if (row<0 || row>=numRows || a1<0 || a1>=numCols)
//            return 1;`) matches the prologue's push/pop set exactly
//            (fixed by caching `numCols` in a local reused for BOTH final
//            `idiv`s, matching `ebx` being loaded once and never reloaded)
//            but stalls at 258B vs 242B: the last bounds check
//            (`a1 < numCols`) always compiles to the INVERSE polarity
//            (`jge`) of the original's `jl`, regardless of whether it is
//            spelled as part of one big `||`, as a chain of nested
//            positive `if`s, or with explicit `goto fail;` for the first
//            three conditions - all three spellings produced byte-identical
//            output, so the compiler's block layout here does not appear
//            to be steerable from this function's own source text. Adding
//            a `goto check_upper;` that skips the (proveably redundant)
//            `row < 0` re-check on the zero-clamp path - mirroring the
//            original's own `jmp` that skips exactly that check - got
//            closest (240B vs 242B, divergence moves later) but the
//            compiler then proves the SAME check dead on the OTHER path
//            too (max-clamp / unclamped), deleting it where the original
//            keeps it; splitting `row<0 || maxRow<0` into two independent
//            `if`s so each one only proves ITS OWN half made this worse,
//            not better. Best landed: the `goto check_upper` version.
//            numCols is cached (`ebx`-shaped); numRows is cached for the
//            bounds check (`edi`-shaped) but read fresh from
//            `*g_00949874` for the final divisions, matching `idiv dword
//            ptr [0x949874]` reloading rather than reusing a register.
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?tile_to_pixel@WorldWin@@QAEHHHPAH0@Z  at 0x004C44E0  (242 bytes)
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
static int *const g_0094988c = (int *)0x0094988C;

class WorldWin { public:
    int tile_to_pixel(int, int, int *, int *);
};
int WorldWin::tile_to_pixel(int a1, int a2, int * a3, int * a4) {
    int numCols = *g_00949870;
    if ((*(unsigned char *)g_0094988c & 1) == 0) {
        if (a1 < 0) {
            a1 = a1 + numCols;
        } else if (a1 >= numCols) {
            a1 = a1 - numCols;
        }
    }
    int numRows = *g_00949874;
    int maxRow = numRows - 1;
    int row = a2;
    if (row < 0 || maxRow < 0) {
        row = 0;
        goto check_upper;
    }
    if (row > maxRow) {
        row = maxRow;
    }
    if (row < 0) goto fail;
check_upper:
    if (row >= numRows) goto fail;
    if (a1 < 0) goto fail;
    if (a1 < numCols) {
        if (*(int *)((char *)this + 0x239c4) == 0 && *(int *)((char *)this + 0x239c8) == 0) {
            *a3 = (*(int *)((char *)this + 0x4c4) * a1) / numCols;
            *a4 = -((*(int *)((char *)this + 0x4c8) * row) / *g_00949874);
            return 0;
        }
        *a3 = (*(int *)(*(int *)(*(int *)((char *)this + 0x152c) + 4) + 0x19f0 + (int)this) * a1) /
              numCols + *(int *)((char *)this + 0x239c4);
        *a4 = *(int *)((char *)this + 0x239c8) -
              (*(int *)(*(int *)(*(int *)((char *)this + 0x152c) + 4) + 0x19f4 + (int)this) * row) /
              *g_00949874;
        return 0;
    }
fail:
    return 1;
}
