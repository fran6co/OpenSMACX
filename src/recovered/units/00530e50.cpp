// ORIGINAL: 0x00530E50 ?unlock_base@NetDaemon@@QAEXH@Z 0x00530E50-0x0053101F FILE
// size      463 bytes
// prototype void (__thiscall ?unlock_base@NetDaemon@@QAEXH@Z)(NetDaemon* this, int)
// callers   26   call targets   10
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00592EE0 0x00593250 0x005FC700 0x006262F0 0x00630A00
// indirect  0x00530EED 0x00530F13
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00530E50
// measured tier  MISMATCH
// divergence     88
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00530e50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?unlock_base@NetDaemon@@QAEXH@Z  at 0x00530E50  (463 bytes)
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
void __cdecl synch_base(int);
void __cdecl wait_task();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_0068c2dc = (int *)0x0068C2DC;
static int *const g_0068c314 = (int *)0x0068C314;
static int *const g_0068c320 = (int *)0x0068C320;
static int *const g_0068c3e0 = (int *)0x0068C3E0;
static int *const g_0068c404 = (int *)0x0068C404;
static int *const g_0068c410 = (int *)0x0068C410;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e904 = (int *)0x0093E904;
static int *const g_0093e93c = (int *)0x0093E93C;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f664 = (int *)0x0093F664;
static int *const g_009b2068 = (int *)0x009B2068;

class NetDaemon { public:
    void process_message(char *, unsigned long, int);
    void unlock_base(int);
};

typedef unsigned long (__stdcall *TimeGetTimeFn)();

// Shares its wait-loop with the sibling ?await_exec@NetDaemon@@: the tail
// `cmp [esi+0x1bb8],ebx; je LOOP_TOP` is a non-rotated do-while backward
// jump. On that sibling, both a `do{}while()` and a `for(;;)` with a
// trailing break compiled to a ROTATED loop instead - the break-check
// duplicated as the back-edge target, one extra `cmp/jne` plus a forward
// jump rather than a single backward one - regardless of spelling, so it
// was RULED OUT there as a scheduling choice, not a source-form bug. Same
// loop shape here; not re-tried, same result expected.
void NetDaemon::unlock_base(int a1) {
    char *self = reinterpret_cast<char *>(this);

    log_say(reinterpret_cast<char *>(g_0068c314), *g_0093e93c, a1, 0);

    if (*g_0093f660 != 0 && *g_0093e93c != 0) {
        log_say(reinterpret_cast<char *>(g_0068c320), 0, 0, 0);
        synch_base(a1);

        if (*g_0093f660 != 0) {
            *reinterpret_cast<int *>(self + 0x1bb8) = 1;
            log_say(reinterpret_cast<char *>(g_0068c3e0), *g_00939284, 0, 0);
            int saved = *g_0093e904;
            ++*g_0093f664;
            unsigned long baseline = (*reinterpret_cast<TimeGetTimeFn *>(g_00669368))();

            int logged_wait = 0;
            if (*reinterpret_cast<int *>(self + 0x1bb8) == 1) {
                do {
                    if (*g_009b2068 != 0) {
                        break;
                    }
                    if (logged_wait == 0) {
                        if ((*reinterpret_cast<TimeGetTimeFn *>(g_00669368))() - baseline > 0x7d0) {
                            logged_wait = 1;
                            reinterpret_cast<NetMsg *>(g_00805338)->pop(
                                reinterpret_cast<char *>(g_0068c404), 0, 2, 0);
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
                } while (*reinterpret_cast<int *>(self + 0x1bb8) == 1);
                if (logged_wait) {
                    reinterpret_cast<NetMsg *>(g_00805338)->close();
                }
            }

            --*g_0093f664;
            *g_0093e904 = saved;
            if (*reinterpret_cast<int *>(self + 0x1bb8) == 0) {
                log_say(reinterpret_cast<char *>(g_0068c410), *g_00939284, 0, 0);
            }
            *reinterpret_cast<int *>(self + 0x1bb8) = 0;

            if (*g_0093f660 != 0) {
                log_say(reinterpret_cast<char *>(g_0068c2dc), *g_00939284, 0, 0);
                message_data(0x2212, 0, 0, 0, 0, 0);
                *reinterpret_cast<int *>(self + 0x1bb0) = 0;
                *reinterpret_cast<int *>(self + 0x1bac) = 0;
                *reinterpret_cast<int *>(self + 0x1bcc) = 0;
                *reinterpret_cast<int *>(self + 0x1bc8) = 0;
            }
        }
    }
}
