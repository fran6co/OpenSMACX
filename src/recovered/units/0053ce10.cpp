// ORIGINAL: 0x0053CE10 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0053CE10
// name           ?make_treaty@@YAXHH@Z
// size           840 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0053ce10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?make_treaty@@YAXHH@Z  at 0x0053CE10  (840 bytes)
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
    void await_diplo(int);
};
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
char * __cdecl get_noun_phrase(int, int);
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
int __cdecl X_pop(char *, const char *, int (__cdecl *)());
int __cdecl X_pops(const char *, int, Sprite *, int (__cdecl *)());
int __cdecl parse_says(int, char *, int, int);
int __cdecl territory(int, int, int, int *, int *);
int __cdecl tut_check2(int);
unsigned int __cdecl random(int, int);
void __cdecl diplomacy_caption(int, int);
void __cdecl diplomacy_check(int, int, int);
void __cdecl log_say(char *, int, int, int);
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl treaty_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_006846d8 = (int *)0x006846D8;
static int *const g_0068d444 = (int *)0x0068D444;
static int *const g_0068d5f4 = (int *)0x0068D5F4;
static int *const g_0068d600 = (int *)0x0068D600;
static int *const g_0068d60c = (int *)0x0068D60C;
static int *const g_00691b14 = (int *)0x00691B14;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f7cc = (int *)0x0093F7CC;
static int *const g_0096ca58 = (int *)0x0096CA58;
static int *const g_0096cb08 = (int *)0x0096CB08;
static int *const g_009a64c4 = (int *)0x009A64C4;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// Per-faction struct array, stride 0x20CC bytes / 2099 ints, indexed by faction id.
extern uint8_t g_0096ca98_tbl[];
extern int     g_0096ca58_tbl[];
extern int     g_0096cb08_tbl[];
// Per-leader struct array, stride 0x59C bytes / 0x167 ints, indexed by leader id.
extern int  g_00946a50_tbl[];
extern char g_00946a9c_tbl[];
extern char g_00946a84_tbl[];
extern int  g_00946d4c_tbl[];
extern int  g_00946d50_tbl[];
extern char g_00946d34_tbl[];
extern char g_00946d54_tbl[];
// Direct-indexed pointer table (by leader id).
extern Sprite *g_006846d8_tbl[];
// The scaffold's `_itoa()` prototype is an empty-parens (no-args) C++
// declaration, so it cannot be called with arguments as declared, and a
// second `extern "C"` declaration for the same symbol is a hard conflict
// (C2733). Reinterpret the existing symbol as the real CRT signature at the
// call site instead of redeclaring it.
typedef char *(__cdecl *ItoaFn)(int, char *, int);

void __cdecl make_treaty(int a1, int a2) {
    *g_0093f7cc = a2;
    if (*g_0093f660 == 0) {
        treaty_on(a2, a1, 0x1000202);
    } else {
        log_say(reinterpret_cast<char *>(g_0068d444), a2, a1, 0x1000202);
        message_data(0x2441, 0, a2, a1, 0x1000202, 0);
        reinterpret_cast<NetDaemon *>(g_0093cd90)->await_diplo(0x441);
    }

    int prior = *g_009a64d4;
    g_0096ca98_tbl[a1 + a2 * 0x20cc] = 0;
    g_0096ca58_tbl[a2 + a1 * 2099] = prior;
    diplomacy_check(a1, a2, 0);
    g_0096cb08_tbl[a1 * 2099]++;
    g_0096cb08_tbl[a2 * 2099]++;
    diplomacy_caption(a1, a2);

    *g_009bbff0 = 0;
    int off1 = a1 * 0x59c;
    *g_009bbfec = g_00946a50_tbl[a1 * 0x167];
    parse_says(0, &g_00946a9c_tbl[off1], -1, -1);
    *g_009bbfec = g_00946a50_tbl[a1 * 0x167];
    *g_009bbff0 = 0;
    parse_says(1, &g_00946a84_tbl[off1], -1, -1);
    *g_009bbfec = g_00946d4c_tbl[a1 * 0x167];
    *g_009bbff0 = g_00946d50_tbl[a1 * 0x167];
    parse_says(2, &g_00946d34_tbl[off1], -1, -1);

    *g_009bbff0 = 0;
    int off2 = a2 * 0x59c;
    *g_009bbfec = g_00946a50_tbl[a2 * 0x167];
    parse_says(3, &g_00946a9c_tbl[off2], -1, -1);
    *g_009bbfec = g_00946a50_tbl[a2 * 0x167];
    *g_009bbff0 = 0;
    parse_says(4, &g_00946a84_tbl[off2], -1, -1);
    *g_009bbff0 = g_00946d50_tbl[a2 * 0x167];
    *g_009bbfec = g_00946d4c_tbl[a2 * 0x167];
    parse_says(5, &g_00946d34_tbl[off2], -1, -1);

    parse_says(6, get_noun_phrase(a1, 9), -1, -1);
    parse_says(7, get_noun_phrase(a2, 9), -1, -1);

    char *buf = reinterpret_cast<char *>(g_009b86a0);
    *buf = 0;
    strcat(buf, reinterpret_cast<char *>(g_0068d5f4));
    unsigned int r = random(0, 2);
    char num[80];
    reinterpret_cast<ItoaFn>(_itoa)(r, num, 10);
    strcat(buf, num);
    X_pops(buf, 0x100000, g_006846d8_tbl[a2],
           reinterpret_cast<int(__cdecl *)()>(g_005398e0));

    if (*g_009a64c4 > 1) {
        if (tut_check2(0x800) == 0)
            goto skip_commerce;
    }
    if (*g_0093f660 == 0) {
        *g_009bbfec = g_00946d4c_tbl[a2 * 0x167];
        *g_009bbff0 = g_00946d50_tbl[a2 * 0x167];
        parse_says(0, &g_00946d34_tbl[off2], -1, -1);
        X_pop(*reinterpret_cast<char **>(g_00691b14),
              reinterpret_cast<const char *>(g_0068d600), 0);
    }
skip_commerce:
    if (territory(a2, a1, 2, 0, 0) != 0) {
        *g_009bbfec = g_00946d4c_tbl[a2 * 0x167];
        *g_009bbff0 = g_00946d50_tbl[a2 * 0x167];
        parse_says(0, &g_00946d34_tbl[off2], -1, -1);
        parse_says(1, &g_00946d54_tbl[off2], -1, -1);
        reinterpret_cast<NetMsg *>(g_00805338)->pop(
            reinterpret_cast<const char *>(g_0068d60c), 5000, 0, 0);
    }
}
