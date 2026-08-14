// ORIGINAL: 0x00531020 FILE
// RULED-OUT: extern char[] base for the runtime *a1*0x34 struct index
//            (shared 0x952828 table with veh_ready); MISMATCH from #34,
//            past the whole early-exit and lock() call - the final
//            store-back sequence around the result branches differs
// working copy - scaffold materialised by --work
// name      ?lock_veh@NetDaemon@@QAEHPAHHHHH@Z
// size      203 bytes
// spans     0x00531020-0x005310EB
// prototype int (__thiscall ?lock_veh@NetDaemon@@QAEHPAHHHHH@Z)(NetDaemon* this, int*, int, int, int, int)
// callers   32   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00530630

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?lock_veh@NetDaemon@@QAEHPAHHHHH@Z  at 0x00531020  (203 bytes)
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

class NetDaemon;
class NetMsg;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068c340 = (int *)0x0068C340;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093e8ec = (int *)0x0093E8EC;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_009a681c = (int *)0x009A681C;

class NetDaemon { public:
    int lock(int, int, int, int, int, int);
    int lock_veh(int *, int, int, int, int);
};
extern char g_952828_arr[];

int NetDaemon::lock_veh(int * a1, int a2, int a3, int a4, int a5) {
    char *self = reinterpret_cast<char *>(this);
    if (*g_0093f660 == 0) {
        return 0;
    }
    if ((*g_0093e8ec & (1 << *g_00939284)) != 0 &&
        (*reinterpret_cast<unsigned char *>(g_009a681c) & 0x10) == 0) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(reinterpret_cast<const char *>(g_0068c340), 0, 2, 0);
        return 1;
    }

    *reinterpret_cast<int **>(self + 0x1b78) = a1;
    char *entry = g_952828_arr + (*a1) * 0x34;
    short y = *reinterpret_cast<short *>(entry + 2);
    short x = *reinterpret_cast<short *>(entry + 0);
    int result = lock(a2, x, y, a5, a3, a4);
    if (result == 0 && *a1 >= 0) {
        *reinterpret_cast<int *>(self + 0x1bb0) = 1;
        *reinterpret_cast<int *>(self + 0x1bc0) = 0;
        return 0;
    }
    *reinterpret_cast<int *>(self + 0x1b78) = 0;
    return 1;
}
