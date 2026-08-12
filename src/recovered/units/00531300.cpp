// ORIGINAL: 0x00531300 FILE
// name      ?await_exec@NetDaemon@@QAEXH@Z
// size      375 bytes
// spans     0x00531300-0x00531477
// prototype void (__thiscall ?await_exec@NetDaemon@@QAEXH@Z)(NetDaemon* this, int)
// callers   17   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00592EE0 0x005FC700 0x006262F0 0x00630A00
// indirect  0x00531345 0x0053136B
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00531300
// measured tier  MISMATCH
// divergence     18
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00531300/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?await_exec@NetDaemon@@QAEXH@Z  at 0x00531300  (375 bytes)
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
void __cdecl check_spock();
void __cdecl log_say(char *, int, int, int);
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl wait_task();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_0068c2dc = (int *)0x0068C2DC;
static int *const g_0068c390 = (int *)0x0068C390;
static int *const g_0068c3b4 = (int *)0x0068C3B4;
static int *const g_0068c3c0 = (int *)0x0068C3C0;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e904 = (int *)0x0093E904;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f664 = (int *)0x0093F664;
static int *const g_009b2068 = (int *)0x009B2068;

class NetDaemon { public:
    void process_message(char *, unsigned long, int);
    void await_exec(int);
};

typedef unsigned long (__stdcall *TimeGetTimeFn)();

void NetDaemon::await_exec(int a1) {
    char *self = reinterpret_cast<char *>(this);

    if (*g_0093f660 == 0) {
        return;
    }

    log_say(reinterpret_cast<char *>(g_0068c390), *g_00939284, 0, 0);
    int saved = *g_0093e904;
    ++*g_0093f664;
    unsigned long baseline = (*reinterpret_cast<TimeGetTimeFn *>(g_00669368))();

    int logged_wait = 0;
    if (*reinterpret_cast<int *>(self + 0x1bb0) == 1) {
        do {
            if (*g_009b2068 != 0) {
                break;
            }
            if (logged_wait == 0) {
                if ((*reinterpret_cast<TimeGetTimeFn *>(g_00669368))() - baseline > 0x7d0) {
                    logged_wait = 1;
                    reinterpret_cast<NetMsg *>(g_00805338)->pop(
                        reinterpret_cast<char *>(g_0068c3b4), 0, 2, 0);
                }
            }
            wait_task();
            unsigned long p1, p2;
            int r = reinterpret_cast<Net *>(g_0093cd90)->get(&p1, &p2);
            if (r != 0) {
                process_message(reinterpret_cast<char *>(r), p2, p1);
            }
            reinterpret_cast<Lock *>(self + 0x14a0)->check_global();
            check_spock();
        } while (*reinterpret_cast<int *>(self + 0x1bb0) == 1);
        if (logged_wait) {
            reinterpret_cast<NetMsg *>(g_00805338)->close();
        }
    }

    --*g_0093f664;
    *g_0093e904 = saved;
    log_say(reinterpret_cast<char *>(g_0068c3c0), *g_00939284, 0, 0);

    if (a1 != 0) {
        if (*g_0093f660 != 0) {
            log_say(reinterpret_cast<char *>(g_0068c2dc), *g_00939284, 0, 0);
            message_data(0x2212, 0, 0, 0, 0, 0);
            *reinterpret_cast<int *>(self + 0x1bb0) = 0;
            *reinterpret_cast<int *>(self + 0x1bac) = 0;
            *reinterpret_cast<int *>(self + 0x1bcc) = 0;
            *reinterpret_cast<int *>(self + 0x1bc8) = 0;
        }
        *reinterpret_cast<int *>(self + 0x1b78) = 0;
        *reinterpret_cast<int *>(self + 0x1bc4) = 0;
    }
    *reinterpret_cast<int *>(self + 0x1bb0) = 0;
}
