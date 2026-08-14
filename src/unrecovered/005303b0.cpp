// ORIGINAL: 0x005303B0 FILE
// working copy - scaffold materialised by --work
// name      ?wait@NetDaemon@@QAEHPADPAHHH@Z
// size      227 bytes
// spans     0x005303B0-0x00530493
// prototype int (__thiscall ?wait@NetDaemon@@QAEHPADPAHHH@Z)(NetDaemon* this, int8*, int*, int, int)
// callers   2   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x005FC700 0x00630A00
// indirect  0x005303CF 0x005303F9

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wait@NetDaemon@@QAEHPADPAHHH@Z  at 0x005303B0  (227 bytes)
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

class Lock;
class Net;
class NetDaemon;
class NetMsg;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Lock { public:
    void check_global();
};

class Net { public:
    int get(unsigned long *, unsigned long *);
};

class NetMsg { public:
    void close();
    void pop(const char *, int, int, const char *);
};

void check_spock();
void wait_task();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e904 = (int *)0x0093E904;
static int *const g_0093f664 = (int *)0x0093F664;
static int *const g_009b2068 = (int *)0x009B2068;

class NetDaemon { public:
    void process_message(char *, unsigned long, int);
    int wait(char *, int *, int, int);
};
int NetDaemon::wait(char * a1, int * a2, int a3, int a4) {
    int saved = *g_0093e904;
    ++*g_0093f664;
    typedef unsigned long (__stdcall *TimeGetTimeFn)();
    TimeGetTimeFn timeGetTimeFn = reinterpret_cast<TimeGetTimeFn>(*g_00669368);
    unsigned long startTime = timeGetTimeFn();
    bool triggeredTimeout = false;
    while (*a2 == a3) {
        if (*g_009b2068 != 0) {
            break;
        }
        if (!triggeredTimeout) {
            unsigned long now = timeGetTimeFn();
            if ((int)(now - startTime) > a4) {
                triggeredTimeout = true;
                reinterpret_cast<NetMsg *>(g_00805338)->pop(a1, 0, 2, 0);
            }
        }
        wait_task();
        unsigned long outVar;
        int result = reinterpret_cast<Net *>(g_0093cd90)->get(reinterpret_cast<unsigned long *>(&a1), &outVar);
        if (result != 0) {
            process_message(reinterpret_cast<char *>(result), outVar, reinterpret_cast<int>(a1));
        }
        reinterpret_cast<Lock *>(reinterpret_cast<char *>(this) + 0x14a0)->check_global();
        check_spock();
    }
    if (triggeredTimeout) {
        reinterpret_cast<NetMsg *>(g_00805338)->close();
    }
    --*g_0093f664;
    *g_0093e904 = saved;
    return 0;
}
