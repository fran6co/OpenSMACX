// ORIGINAL: 0x0053E5C0 FILE
// name      ?renounce_pact@@YAXHH@Z
// size      568 bytes
// spans     0x0053E5C0-0x0053E7F8
// prototype 
// callers   1   call targets   15
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0051F1B0 0x00530AA0 0x00530D00 0x005315C0 0x005399A0 0x0053A980 0x0053C650 0x005589E0 0x0055B760 0x0058F9D0 0x00592EE0 0x005BF310 0x005BF7D0 0x00625EC0 0x006262F0
// indirect  0x0053E5F7 0x0053E61B 0x0053E652
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0053E5C0
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0053e5c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?renounce_pact@@YAXHH@Z  at 0x0053E5C0  (568 bytes)
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
class Sprite;

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
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int add_global();
    void await_diplo(int);
    void unlock();
};
char * __cdecl get_pact(int);
int __cdecl X_pop(const char *, int (__cdecl *)());
int __cdecl X_pops(const char *, int, Sprite *, int (__cdecl *)());
int __cdecl commlink_attempter(int, int);
int __cdecl parse_says(int, char *, int, int);
void __cdecl check_spock();
void __cdecl diplomacy_caption(int, int);
void __cdecl diplomacy_check(int, int, int);
void __cdecl log_say(char *, int, int, int);
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl pact_ends(int, int);
void __cdecl treaty_off(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_006846d8 = (int *)0x006846D8;
static int *const g_0068d458 = (int *)0x0068D458;
static int *const g_0068d500 = (int *)0x0068D500;
static int *const g_0068d554 = (int *)0x0068D554;
static int *const g_0068d56c = (int *)0x0068D56C;
static int *const g_0068d578 = (int *)0x0068D578;
static int *const g_0068d698 = (int *)0x0068D698;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f7cc = (int *)0x0093F7CC;
static int *const g_0096cb04 = (int *)0x0096CB04;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a6800 = (int *)0x009A6800;
static int *const g_009b2068 = (int *)0x009B2068;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// Indexed table bases: the pointer-constant spelling for these three
// globals folds the scaled-index addressing under /O2 because the ADDRESS
// ITSELF does the index arithmetic; declare array forms instead.
extern int g_0096cb04_arr[];
extern void *g_006846d8_arr[];
extern char g_00946a50_arr[];

typedef unsigned long(__stdcall *TimeGetTimeFn)();

void __cdecl renounce_pact(int a1, int a2) {
    *g_0093f7cc = a2;
    if (commlink_attempter(a1, a2) != 0) {
        return;
    }

    log_say(reinterpret_cast<char *>(g_0068d554), 0, 0, 0);

    TimeGetTimeFn timeGetTimeFn = *reinterpret_cast<TimeGetTimeFn *>(g_00669368);
    unsigned long t0 = timeGetTimeFn();
    int counter = 0;
    while (*g_009b2068 == 0 &&
           reinterpret_cast<NetDaemon *>(g_0093cd90)->add_global() != 0) {
        check_spock();
        unsigned long t1 = timeGetTimeFn();
        if (t1 - t0 >= 10000 || ++counter >= 10000) {
            if (X_pop(reinterpret_cast<const char *>(g_0068d56c), 0) != 0) {
                return;
            }
            t0 = timeGetTimeFn();
            counter = 0;
        }
    }

    g_0096cb04_arr[a1 * 2099]++;

    if (*g_0093f660 == 0) {
        treaty_off(a2, a1, 1);
    } else {
        log_say(reinterpret_cast<char *>(g_0068d458), a2, a1, 1);
        message_data(0x2442, 0, a2, a1, 1, 0);
        reinterpret_cast<NetDaemon *>(g_0093cd90)->await_diplo(0x442);
    }

    if (*g_009a6800 <= 0 &&
        (*reinterpret_cast<unsigned char *>(g_009a64e8) & (1 << (a2 & 0x1f))) == 0) {
        diplomacy_check(a1, a2, 0);
        diplomacy_caption(a1, a2);

        char *pact = get_pact(a1);
        parse_says(0, pact, -1, -1);

        char *rec = g_00946a50_arr + a1 * 1436;
        *g_009bbfec = *reinterpret_cast<int *>(rec);
        *g_009bbff0 = 0;
        parse_says(1, rec + 0x4c, -1, -1);

        *g_009bbfec = *reinterpret_cast<int *>(rec);
        *g_009bbff0 = 0;
        parse_says(2, rec + 0x34, -1, -1);

        void *sprite = g_006846d8_arr[a2];
        X_pops(reinterpret_cast<const char *>(g_0068d698), 0x100000,
               reinterpret_cast<Sprite *>(sprite),
               reinterpret_cast<int(__cdecl *)()>(g_005398e0));
    }

    if (*g_0093f660 == 0) {
        pact_ends(a1, a2);
    } else {
        log_say(reinterpret_cast<char *>(g_0068d500), 0, 0, 0);
        message_data(0x244d, 0, a1, a2, 0, 0);
        reinterpret_cast<NetDaemon *>(g_0093cd90)->await_diplo(0x44d);
    }

    log_say(reinterpret_cast<char *>(g_0068d578), 0, 0, 0);
    reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock();
}
