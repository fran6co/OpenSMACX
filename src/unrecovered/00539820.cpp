// ORIGINAL: 0x00539820 BYTE_EXACT FILE
// LEVER: `x >= 10000` (not `9999 < x`) on both the elapsed-time and
//        retry-count checks - each `9999 < x` spelling compiled to a
//        signed/short-circuited jg/ja instead of the original's jae/jl.
// working copy - scaffold materialised by --work
// name      ?diplo_lock@@YAHH@Z
// size      159 bytes
// spans     0x00539820-0x005398BF
// prototype 
// callers   10   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0051F1B0 0x00530AA0 0x005BF310 0x006262F0
// indirect  0x00539851 0x00539871 0x005398A1

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?diplo_lock@@YAHH@Z  at 0x00539820  (159 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int add_global();
};

int __cdecl X_pop(const char *, int (__cdecl *)());
void check_spock();
void log_say(char *, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_0068d554 = (int *)0x0068D554;
static int *const g_0068d56c = (int *)0x0068D56C;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_009b2068 = (int *)0x009B2068;
typedef unsigned long (__stdcall *TimeGetTimeFn)();

int __cdecl diplo_lock(int a1) {
    int retry_count = 0;
    log_say(reinterpret_cast<char *>(g_0068d554), 0, 0, 0);
    if (a1 != 0 && a1 < 100) {
        a1 = 100;
    }

    TimeGetTimeFn get_time = *reinterpret_cast<TimeGetTimeFn *>(g_00669368);

    unsigned long start_time = get_time();
    while (*g_009b2068 == 0 && reinterpret_cast<NetDaemon *>(g_0093cd90)->add_global() != 0) {
        check_spock();
        unsigned long now = get_time();
        if (a1 != 0 && (now - start_time >= 10000 || (++retry_count, retry_count >= 10000))) {
            int need_global = X_pop(reinterpret_cast<const char *>(g_0068d56c), 0);
            if (need_global != 0) {
                return 1;
            }
            retry_count = 0;
            start_time = get_time();
        }
    }
    return 0;
}
