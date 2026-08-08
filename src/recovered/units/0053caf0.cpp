// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0053CAF0
// name           ?pact_of_brotherhood@@YAXHH@Z
// size           789 bytes
// measured tier  MISMATCH
// divergence     13
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0053caf0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pact_of_brotherhood@@YAXHH@Z  at 0x0053CAF0  (789 bytes)
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
    void await_diplo(int);
};
char * __cdecl get_pact(int);
char * __cdecl get_phrase(int, int);
int __cdecl X_pop(char *, const char *, int (__cdecl *)());
int __cdecl X_pops(const char *, int, Sprite *, int (__cdecl *)());
int __cdecl parse_says(int, char *, int, int);
int __cdecl tut_check2(int);
void __cdecl diplomacy_caption(int, int);
void __cdecl diplomacy_check(int, int, int);
void __cdecl log_say(char *, int, int, int);
void __cdecl mention_prototypes(int, int);
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl scan_prototypes(int, int);
void __cdecl trade_maps(int, int);
void __cdecl treaty_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_006846d8 = (int *)0x006846D8;
static int *const g_0068d444 = (int *)0x0068D444;
static int *const g_0068d4dc = (int *)0x0068D4DC;
static int *const g_0068d5d4 = (int *)0x0068D5D4;
static int *const g_0068d5e0 = (int *)0x0068D5E0;
static int *const g_0068d5ec = (int *)0x0068D5EC;
static int *const g_00691b14 = (int *)0x00691B14;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f7cc = (int *)0x0093F7CC;
static int *const g_0093fa70 = (int *)0x0093FA70;
static int *const g_0096ca58 = (int *)0x0096CA58;
static int *const g_0096cb08 = (int *)0x0096CB08;
static int *const g_009a64c4 = (int *)0x009A64C4;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

extern uint8_t g_0096ca98[];
extern char g_00946a50[];
extern char g_00946a84[];
extern char g_00946a9c[];
extern char g_00946d34[];
extern char g_00946d4c[];
extern char g_00946d50[];
extern char g_00946d54[];

void __cdecl pact_of_brotherhood(int a1, int a2) {
    if (*g_0093f660 == 0) {
        *g_0093f7cc = a2;
        treaty_on(a2, a1, 0x1000001);
    } else {
        *g_0093f7cc = a2;
        log_say(reinterpret_cast<char *>(g_0068d444), a2, a1, 0x1000001);
        message_data(0x2441, 0, a2, a1, 0x1000001, 0);
    }

    int32_t v = *g_009a64d4;
    int32_t off1 = a2 * 0x20cc;
    g_0096ca98[a1 + off1] = 0;
    *reinterpret_cast<int32_t *>(reinterpret_cast<char *>(g_0096ca58) + a2 * 4 + a1 * 0x20cc) = v;
    diplomacy_check(a1, a2, 0);
    *g_0093fa70 = 0;
    ++*reinterpret_cast<int32_t *>(reinterpret_cast<char *>(g_0096cb08) + a1 * 0x20cc);
    ++*reinterpret_cast<int32_t *>(reinterpret_cast<char *>(g_0096cb08) + off1);
    diplomacy_caption(a1, a2);

    *g_009bbff0 = 0;
    int32_t off2 = a2 * 0x59c;
    *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946a50 + off2);
    parse_says(0, g_00946a9c + off2, -1, -1);
    *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946a50 + off2);
    *g_009bbff0 = 0;
    parse_says(1, g_00946a84 + off2, -1, -1);
    *g_009bbff0 = *reinterpret_cast<int32_t *>(g_00946d50 + off2);
    *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946d4c + off2);
    parse_says(2, g_00946d34 + off2, -1, -1);

    parse_says(3, get_pact(a1), -1, -1);
    parse_says(4, get_phrase(a1, 0), -1, -1);
    parse_says(5, get_phrase(a2, 0), -1, -1);

    X_pops(reinterpret_cast<const char *>(g_0068d5d4), 0x100000,
           *reinterpret_cast<Sprite **>(reinterpret_cast<char *>(g_006846d8) + a2 * 4),
           reinterpret_cast<int (__cdecl *)()>(g_005398e0));

    if (*g_0093f660 == 0) {
        trade_maps(a1, a2);
    } else {
        log_say(reinterpret_cast<char *>(g_0068d4dc), a1, a2, 0);
        message_data(0x2448, 0, a1, a2, 0, 0);
        reinterpret_cast<NetDaemon *>(0x93cd90)->await_diplo(0x448);
    }

    parse_says(0, get_pact(a1), -1, -1);
    X_pops(reinterpret_cast<const char *>(g_0068d5e0), 0x100000,
           *reinterpret_cast<Sprite **>(reinterpret_cast<char *>(g_006846d8) + a2 * 4),
           reinterpret_cast<int (__cdecl *)()>(g_005398e0));
    scan_prototypes(a1, a2);
    mention_prototypes(a1, a2);

    if (*g_009a64c4 > 1 && tut_check2(0x1000) == 0) {
        return;
    }

    if (*g_0093f660 == 0) {
        *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946d4c + off2);
        *g_009bbff0 = *reinterpret_cast<int32_t *>(g_00946d50 + off2);
        parse_says(0, g_00946d34 + off2, -1, -1);
        *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946a50 + off2);
        *g_009bbff0 = 0;
        parse_says(1, g_00946a9c + off2, -1, -1);
        *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946a50 + off2);
        *g_009bbff0 = 0;
        parse_says(2, g_00946a84 + off2, -1, -1);
        parse_says(3, g_00946d54 + off2, -1, -1);
        X_pop(*reinterpret_cast<char **>(g_00691b14), reinterpret_cast<const char *>(g_0068d5ec), 0);
    }
}
