// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00545D60
// name           ?propose_trade_maps@@YAXHH@Z
// size           288 bytes
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00545d60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?propose_trade_maps@@YAXHH@Z  at 0x00545D60  (288 bytes)
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
int __cdecl X_pops(const char *, int, Sprite *, int (__cdecl *)());
int __cdecl parse_says(int, char *, int, int);
void __cdecl log_say(char *, int, int, int);
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl trade_maps(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005398e0 = (int *)0x005398E0;
// Table bases: the address itself is indexed (`[reg*4 + base]`), so a
// folded literal would lose the addressing mode. Declared as arrays instead.
extern Sprite *g_006846d8[];
static int *const g_0068d4dc = (int *)0x0068D4DC;
static int *const g_0068da60 = (int *)0x0068DA60;
static int *const g_0068da68 = (int *)0x0068DA68;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f7cc = (int *)0x0093F7CC;
static int *const g_0093fa74 = (int *)0x0093FA74;
static int *const g_0093fad8 = (int *)0x0093FAD8;
extern uint8_t g_00946d54[];
extern uint8_t g_0096c9f8[];

void __cdecl propose_trade_maps(int a1, int a2) {
    *g_0093f7cc = a2;

    int flag = g_0096c9f8[a2 * 4 + a1 * 0x20CC] & 1;
    int turn = *g_0093fa74;
    if (flag == 0 && turn > 0) {
        goto reject;
    }
    if (turn <= 4) {
        X_pops(reinterpret_cast<const char *>(g_0068da68), 0x100000,
               g_006846d8[a2], reinterpret_cast<int (__cdecl *)()>(g_005398e0));

        reinterpret_cast<uint8_t *>(0x0096CA98)[a2 * 0x20CC + a1] += 2;

        if (*g_0093f660 != 0) {
            log_say(reinterpret_cast<char *>(g_0068d4dc), a1, a2, 0);
            message_data(0x2448, 0, a1, a2, 0, 0);
            reinterpret_cast<NetDaemon *>(g_0093cd90)->await_diplo(0x448);
            *g_0093fad8 = 1;
            return;
        }

        trade_maps(a1, a2);
        *g_0093fad8 = 1;
        return;
    }

reject:
    parse_says(0, reinterpret_cast<char *>(g_00946d54) + a2 * 0x59C, -1, -1);
    X_pops(reinterpret_cast<const char *>(g_0068da60), 0x100000,
           g_006846d8[a2], reinterpret_cast<int (__cdecl *)()>(g_005398e0));
}
