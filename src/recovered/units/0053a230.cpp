// ORIGINAL: 0x0053A230 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0053A230
// name           ?mention_prototypes@@YAXHH@Z
// size           614 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0053a230/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mention_prototypes@@YAXHH@Z  at 0x0053A230  (614 bytes)
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
extern "C" char *strcat(char *, const char *);
int __cdecl X_pops(const char *, int, Sprite *, int (__cdecl *)());
int __cdecl parse_says(int, char *, int, int);
int __cdecl wants_prototype(int, int);
int __cdecl wants_to_speak(int, int);
void __cdecl log_say(char *, int, int, int);
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl say_stats_2(char *, int);
void __cdecl tech_achieved(int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_006846d8 = (int *)0x006846D8;
static int *const g_0068d4f0 = (int *)0x0068D4F0;
static int *const g_0068d598 = (int *)0x0068D598;
static int *const g_0068d5a0 = (int *)0x0068D5A0;
static int *const g_0068d5a8 = (int *)0x0068D5A8;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f7cc = (int *)0x0093F7CC;
static int *const g_0093f804 = (int *)0x0093F804;
static int *const g_0093fa40 = (int *)0x0093FA40;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_009ab898 = (int *)0x009AB898;
static int *const g_009b86a0 = (int *)0x009B86A0;

// EXTERN-SYMBOL LEVER: each of these addresses is combined with a
// runtime-computed offset (a table index scaled by a register), not
// read/written plainly, so a pointer-constant folds under /O2 and the
// indexed addressing form the original used disappears with it.
extern uint8_t g_009ab898x[];
extern uint8_t g_009ab868x[];
extern int g_0096c9f8x[];
extern Sprite *g_006846d8x[];

void __cdecl mention_prototypes(int a1, int a2) {
    int netStatus = *g_0093fa40;
    *g_0093f7cc = a2;
    if (netStatus < 0) {
        return;
    }
    if (!wants_to_speak(a1, a2)) {
        return;
    }

    uint8_t *a2Block = g_009ab898x + a2 * 0xD00;
    {
        uint8_t *p = a2Block;
        int n = 0x40;
        do {
            *p |= 0x10;
            p += 0x34;
        } while (--n);
    }

    int netIdx = *g_0093fa40;
    int off34 = netIdx * 0x34;
    parse_says(0, reinterpret_cast<char *>(g_009ab868x + off34), -1, -1);

    char *msgBuf = reinterpret_cast<char *>(0x9b86a0);
    *msgBuf = 0;
    say_stats_2(msgBuf, netIdx);
    parse_says(1, msgBuf, -1, -1);

    uint8_t *netRec = g_009ab898x + off34;
    *netRec |= 0x10;
    *msgBuf = 0;
    strcat(msgBuf, reinterpret_cast<char *>(g_0068d598));

    int flagIdx = a2 + a1 * 2099;
    if (g_0096c9f8x[flagIdx] & 1) {
        *netRec |= 0x20;
        strcat(msgBuf, reinterpret_cast<char *>(g_0068d5a0));
        return;
    }

    strcat(msgBuf, reinterpret_cast<char *>(g_0068d5a8));

    Sprite *sprite = g_006846d8x[a2];
    if (X_pops(msgBuf, 0x100000, sprite, reinterpret_cast<int (__cdecl *)()>(g_005398e0))) {
        int iVar2 = a1 * 0x40 + 0x61;
        int iVar5 = a2 * 0x40 + 0x61;
        uint8_t *a1Block = g_009ab898x + a1 * 0xD00;
        uint8_t *a2Walk = a2Block;
        int n = 0x40;
        do {
            if (wants_prototype(iVar2 - 0x61, a2)) {
                if (*g_0093f660 == 0) {
                    tech_achieved(a2, iVar2, a1, 0);
                } else {
                    log_say(reinterpret_cast<char *>(g_0068d4f0), a2, iVar2, a1);
                    message_data(0x244b, 0, a2, iVar2, a1, 0);
                    reinterpret_cast<NetDaemon *>(g_0093cd90)->await_diplo(0x44b);
                }
            }
            *a1Block |= 0x20;

            if (wants_prototype(iVar5 - 0x61, a1)) {
                if (*g_0093f660 == 0) {
                    tech_achieved(a1, iVar5, a2, 0);
                } else {
                    log_say(reinterpret_cast<char *>(g_0068d4f0), a1, iVar5, a2);
                    message_data(0x244b, 0, a1, iVar5, a2, 0);
                    reinterpret_cast<NetDaemon *>(g_0093cd90)->await_diplo(0x44b);
                }
            }
            *a2Walk |= 0x20;

            a1Block += 0x34;
            a2Walk += 0x34;
            ++iVar2;
            ++iVar5;
        } while (--n);
    }

    *g_0093f804 = -1;
    *g_0093fa40 = -1;
}
