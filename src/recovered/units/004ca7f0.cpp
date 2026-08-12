// ORIGINAL: 0x004CA7F0 FILE
// name      ?action_staple@@YAXH@Z
// size      603 bytes
// spans     0x004CA7F0-0x004CAA4B
// prototype void (__cdecl ?action_staple@@YAXH@Z)(int baseID)
// callers   3   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AF40 0x0047A890 0x0048C0A0 0x0055EEE0 0x00625EC0 0x0064601D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004CA7F0
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004ca7f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action_staple@@YAXH@Z  at 0x004CA7F0  (603 bytes)
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

class NetMsg;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
extern "C" int rand();
int __cdecl parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
void __cdecl atrocity(int, int, int, int);
void __cdecl draw_tile(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00687c84 = (int *)0x00687C84;
static int *const g_00687c94 = (int *)0x00687C94;
static int *const g_00687ca4 = (int *)0x00687CA4;
static int *const g_00687cb4 = (int *)0x00687CB4;
static int *const g_00687cc4 = (int *)0x00687CC4;
static int *const g_00687cd4 = (int *)0x00687CD4;
static int *const g_00687ce4 = (int *)0x00687CE4;
static int *const g_00687cf4 = (int *)0x00687CF4;
static int *const g_00687d04 = (int *)0x00687D04;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

extern "C" int __cdecl rand();

static char *const g_0097d040 = (char *)0x0097D040;
static char *const g_0097d042 = (char *)0x0097D042;
static char *const g_0097d044 = (char *)0x0097D044;
static char *const g_0097d045 = (char *)0x0097D045;
static char *const g_0097d048 = (char *)0x0097D048;
static char *const g_0097d053 = (char *)0x0097D053;
static char *const g_0097d070 = (char *)0x0097D070;
static char *const g_0097d168 = (char *)0x0097D168;
static char *const g_00946a50 = (char *)0x00946A50;
static char *const g_00946a84 = (char *)0x00946A84;
static char *const g_00946a9c = (char *)0x00946A9C;

void __cdecl action_staple(int a1) {
    int off = a1 * 0x134;

    int *counter_field = reinterpret_cast<int *>(g_0097d168 + off);
    int attempts = *counter_field + 1;
    *counter_field = attempts;

    unsigned char faction = *reinterpret_cast<unsigned char *>(g_0097d044 + off);

    bool go_success = false;
    if (attempts <= 2) {
        go_success = true;
    } else if (attempts <= 8) {
        if ((rand() % 2) != 0) {
            go_success = true;
        }
    }

    if (go_success) {
        *reinterpret_cast<int *>(g_0097d070 + off) &= ~2;
        *reinterpret_cast<char *>(g_0097d048 + off) += 10;
        draw_tile(*reinterpret_cast<short *>(g_0097d040 + off),
                  *reinterpret_cast<short *>(g_0097d042 + off), 2);

        faction = *reinterpret_cast<unsigned char *>(g_0097d044 + off);
        if (faction == *g_00939284) {
            parse_says(0, g_0097d053 + off, -1, -1);
            if ((*reinterpret_cast<unsigned char *>(g_00946f58 + faction * 0x59c) & 0x80) == 0) {
                popp(reinterpret_cast<char *>(*g_00691b0c),
                     reinterpret_cast<const char *>(g_00687cf4), 0,
                     reinterpret_cast<const char *>(g_00687ce4), 0);
            } else {
                popp(reinterpret_cast<char *>(*g_00691b0c),
                     reinterpret_cast<const char *>(g_00687cd4), 0,
                     reinterpret_cast<const char *>(g_00687cc4), 0);
            }
        } else {
            parse_says(0, g_0097d053 + off, -1, -1);
            *g_009bbff0 = 0;
            *g_009bbfec = *reinterpret_cast<int *>(g_00946a50 + faction * 0x59c);
            parse_says(1, g_00946a9c + faction * 0x59c, -1, -1);
            *g_009bbff0 = 0;
            *g_009bbfec = *reinterpret_cast<int *>(g_00946a50 + faction * 0x59c);
            parse_says(2, g_00946a84 + faction * 0x59c, -1, -1);
            reinterpret_cast<NetMsg *>(g_00805338)->pop(
                reinterpret_cast<const char *>(g_00687d04), 0x1388, 0, 0);
        }
    } else {
        if (faction == *g_00939284) {
            parse_says(0, g_0097d053 + off, -1, -1);
            if ((*reinterpret_cast<unsigned char *>(g_00946f58 + faction * 0x59c) & 0x80) == 0) {
                popp(reinterpret_cast<char *>(*g_00691b0c),
                     reinterpret_cast<const char *>(g_00687cb4), 0,
                     reinterpret_cast<const char *>(g_00687ca4), 0);
            } else {
                popp(reinterpret_cast<char *>(*g_00691b0c),
                     reinterpret_cast<const char *>(g_00687c94), 0,
                     reinterpret_cast<const char *>(g_00687c84), 0);
            }
        }
    }

    unsigned char b45 = *reinterpret_cast<unsigned char *>(g_0097d045 + off);
    unsigned char b44 = *reinterpret_cast<unsigned char *>(g_0097d044 + off);
    atrocity(b44, b45, b45 != b44, 1);
}
