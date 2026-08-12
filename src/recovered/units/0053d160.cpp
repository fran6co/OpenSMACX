// ORIGINAL: 0x0053D160 FILE
// name      ?pledge_truce@@YAXHHH@Z
// size      497 bytes
// spans     0x0053D160-0x0053D351
// prototype 
// callers   1   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005399A0 0x0053A980 0x0055B870 0x00592EE0 0x005BF7D0 0x00625EC0 0x006262F0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0053D160
// measured tier  MISMATCH
// divergence     85
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0053d160/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pledge_truce@@YAXHHH@Z  at 0x0053D160  (497 bytes)
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
int __cdecl X_pops(const char *, int, Sprite *, int (__cdecl *)());
int __cdecl parse_says(int, char *, int, int);
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
static int *const g_0068d618 = (int *)0x0068D618;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f7cc = (int *)0x0093F7CC;
static int *const g_0096ca58 = (int *)0x0096CA58;
static int *const g_0096cb08 = (int *)0x0096CB08;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// Indexed-table variants of context globals that need base+scaled-register
// addressing rather than a folded pointer constant.
extern int32_t g_0096ca58_tbl[];
extern int32_t g_0096cb08_tbl[];
extern void *g_006846d8_tbl[];

// Per-faction record table, stride 0x59c bytes. Not in the context's fixed-
// global list at all (only ever touched here). Reached as a byte base with
// explicit per-field byte offsets - matching Ghidra's own `iVar1 = a1*0x59c`
// byte-offset framing - rather than as a scaled int32 array: the array-index
// spelling let /O2 fold the record stride into a `[reg*4+const]` addressing
// mode at every access and drop the original's separate `shl edi,2`.
extern uint8_t g_00946a50[];

void __cdecl pledge_truce(int a1, int a2, int a3) {
    int flag = *g_0093f660;
    *g_0093f7cc = a2;
    if (flag != 0) {
        log_say(reinterpret_cast<char *>(g_0068d444), a2, a1, 0x204);
        message_data(0x2441, 0, a2, a1, 0x204, 0);
    } else {
        treaty_on(a2, a1, 0x204);
    }

    g_0096ca58_tbl[a2 + a1 * 2099] = *g_009a64d4;
    diplomacy_check(a1, a2, 0);

    g_0096cb08_tbl[a1 * 2099]++;
    g_0096cb08_tbl[a2 * 2099]++;

    if (a3 != 0) {
        diplomacy_caption(a1, a2);

        int rec1 = a1 * 0x59c;
        *g_009bbff0 = 0;
        *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946a50 + rec1);
        parse_says(0, reinterpret_cast<char *>(g_00946a50 + rec1 + 0x4c), -1, -1);

        *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946a50 + rec1);
        *g_009bbff0 = 0;
        parse_says(1, reinterpret_cast<char *>(g_00946a50 + rec1 + 0x34), -1, -1);

        *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946a50 + rec1 + 0x2fc);
        *g_009bbff0 = *reinterpret_cast<int32_t *>(g_00946a50 + rec1 + 0x300);
        parse_says(2, reinterpret_cast<char *>(g_00946a50 + rec1 + 0x2e4), -1, -1);

        int rec2 = a2 * 0x59c;
        *g_009bbff0 = 0;
        *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946a50 + rec2);
        parse_says(3, reinterpret_cast<char *>(g_00946a50 + rec2 + 0x4c), -1, -1);

        *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946a50 + rec2);
        *g_009bbff0 = 0;
        parse_says(4, reinterpret_cast<char *>(g_00946a50 + rec2 + 0x34), -1, -1);

        *g_009bbfec = *reinterpret_cast<int32_t *>(g_00946a50 + rec2 + 0x2fc);
        *g_009bbff0 = *reinterpret_cast<int32_t *>(g_00946a50 + rec2 + 0x300);
        parse_says(5, reinterpret_cast<char *>(g_00946a50 + rec2 + 0x2e4), -1, -1);

        X_pops(reinterpret_cast<const char *>(g_0068d618), 0x100000,
               reinterpret_cast<Sprite *>(g_006846d8_tbl[a2]),
               reinterpret_cast<int (__cdecl *)()>(g_005398e0));
    }
}
