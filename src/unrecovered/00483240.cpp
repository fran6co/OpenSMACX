// ORIGINAL: 0x00483240 ?modal_callback_daemon@NetWin@@QAAHXZ 0x00483240-0x004832CF FILE
// RULED-OUT: do/while (empty body) instead of while - the inner `if` already proved the first iteration, so the loop itself is a do/while; `while` produced an extra redundant pre-test call (18B over)
// RULED-OUT: duplicating `*g_00811d10=1;` into both sides of the count<1 check to match the store landing before `jl` in the original - regressed to an earlier divergence; left as single store (4B over, one instruction reordered around the branch)
// working copy - scaffold materialised by --work
// size      143 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00530320
// indirect  0x00483274

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?modal_callback_daemon@NetWin@@QAAHXZ  at 0x00483240  (143 bytes)
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
class NetWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int receive();
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0080a6f8 = (int *)0x0080A6F8;
static int *const g_00811d0c = (int *)0x00811D0C;
static int *const g_00811d10 = (int *)0x00811D10;
static int *const g_0090dd15 = (int *)0x0090DD15;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f8 = (int *)0x0093D4F8;
static int *const g_009b2068 = (int *)0x009B2068;

class NetWin { public:
    int __cdecl modal_callback_daemon();
};
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
};

int __cdecl NetWin::modal_callback_daemon() {
    NetDaemon *daemon = reinterpret_cast<NetDaemon *>(g_0093cd90);
    if (daemon->receive() != 0) {
        if (daemon->receive() != 0) {
            do {
            } while (daemon->receive() != 0);
        }
        reinterpret_cast<VCall *>(g_0080a6f8)->slot012();
    }
    if (*g_00811d0c == 0) {
        int count = *g_0093d4f8;
        *g_00811d10 = 1;
        if (count >= 1) {
            char *p = reinterpret_cast<char *>(g_0090dd15);
            int found_empty = 1;
            do {
                if (*p == 0) {
                    found_empty = 0;
                }
                p += 0x17c;
                count--;
            } while (count != 0);
            *g_00811d10 = found_empty;
            if (found_empty != 0) {
                *g_00811d0c = 1;
                return 0;
            }
            if (*g_009b2068 != 0) {
                return 0;
            }
            return 1;
        }
        *g_00811d0c = 1;
    }
    return 0;
}
