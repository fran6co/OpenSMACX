// ORIGINAL: 0x00531700 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      ?maybe_lock@NetDaemon@@QAEHPAHHHHHH@Z
// size      128 bytes
// spans     0x00531700-0x00531780
// prototype int (__thiscall ?maybe_lock@NetDaemon@@QAEHPAHHHHHH@Z)(NetDaemon* this, int*, int, int, int, int, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00531020

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?maybe_lock@NetDaemon@@QAEHPAHHHHHH@Z  at 0x00531700  (128 bytes)
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
static int *const g_0068c474 = (int *)0x0068C474;
static int *const g_00805338 = (int *)0x00805338;

class NetDaemon { public:
    int lock_veh(int *, int, int, int, int);
    int maybe_lock(int *, int, int, int, int, int);
};
int NetDaemon::maybe_lock(int * a1, int a2, int a3, int a4, int a5, int a6) {
    char *self = reinterpret_cast<char *>(this);
    if (a2 != 0) {
        return 0;
    }
    if (*reinterpret_cast<int *>(self + 0x1bac) != 0) {
        if (*reinterpret_cast<int *>(self + 0x1bc4) == 0) {
            int *pCount = reinterpret_cast<int *>(self + 0x1bcc);
            if (1 < ++(*pCount)) {
                reinterpret_cast<NetMsg *>(g_00805338)->pop((const char *)g_0068c474, 0, 2, 0);
            }
        }
        return 1;
    }
    *reinterpret_cast<int *>(self + 0x1bcc) = 0;
    return lock_veh(a1, a3 | 4, a4, a5, a6);
}
