// ORIGINAL: 0x004F5CE0 FILE
// name      ?base_drones@@YAXXZ
// size      581 bytes
// spans     0x004F5CE0-0x004F5F25
// prototype 
// callers   1   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046AF40 0x0048C650 0x004E5FE0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004F5CE0
// measured tier  MISMATCH
// divergence     18
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004f5ce0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_drones@@YAXXZ  at 0x004F5CE0  (581 bytes)
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
int __cdecl popb(const char *, int, int, const char *, Sprite *);
void __cdecl base_reset(int, int);
void __cdecl draw_tile(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00689370 = (int *)0x00689370;
static int *const g_00689c78 = (int *)0x00689C78;
static int *const g_00689c84 = (int *)0x00689C84;
static int *const g_00689c90 = (int *)0x00689C90;
static int *const g_00689ca0 = (int *)0x00689CA0;
static int *const g_00689cac = (int *)0x00689CAC;
static int *const g_00689cb8 = (int *)0x00689CB8;
static int *const g_00689cc8 = (int *)0x00689CC8;
static int *const g_00689cd8 = (int *)0x00689CD8;
static int *const g_00689ce8 = (int *)0x00689CE8;
static int *const g_00689cf4 = (int *)0x00689CF4;
static int *const g_00689d04 = (int *)0x00689D04;
static int *const g_00689d10 = (int *)0x00689D10;
static int *const g_00689d20 = (int *)0x00689D20;
static int *const g_00689d30 = (int *)0x00689D30;
static int *const g_00689d40 = (int *)0x00689D40;
static int *const g_00689d50 = (int *)0x00689D50;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a658c = (int *)0x009A658C;

// Indexed table bases the emitter left as plain pointers: both are read
// through a register-scaled address (`[reg*4 + ADDR]`), which /O2 folds a
// plain-pointer spelling away entirely.
extern unsigned char g_00946f58_tbl[];
extern unsigned char g_0097d044_tbl[];

// Reached only through a tail `jmp`, so the static analysis that populates
// the callee list never saw it as a call.
void __cdecl drone_riot();

void __cdecl base_drones() {
    char *base = reinterpret_cast<char *>(*g_0090ea30);
    int faction = static_cast<unsigned char>(base[4]);
    int drones = *reinterpret_cast<int *>(base + 0x11c);

    if (drones == 0 && base[6] > 2 &&
        *reinterpret_cast<int *>(base + 0x118) >= (base[6] + 1) / 2) {
        if ((base[0x30] & 4) == 0) {
            const char *msgA;
            const char *msgB;
            if ((g_00946f58_tbl[faction * 0x59c] & 0x80) == 0) {
                msgA = reinterpret_cast<const char *>(g_00689c90);
                msgB = reinterpret_cast<const char *>(g_00689ca0);
            } else {
                msgA = reinterpret_cast<const char *>(g_00689c78);
                msgB = reinterpret_cast<const char *>(g_00689c84);
            }
            popb(msgB, 0x20, -1, msgA, 0);
            base = reinterpret_cast<char *>(*g_0090ea30);
        }
        base[0x30] = base[0x30] | 4;
    } else if ((base[0x30] & 4) != 0) {
        base[0x30] = base[0x30] & 0xfb;
        base = reinterpret_cast<char *>(*g_0090ea30);
        if (*reinterpret_cast<int *>(base + 0x11c) <= *reinterpret_cast<int *>(base + 0x118)) {
            faction = static_cast<unsigned char>(base[4]);
            const char *msgA;
            const char *msgB;
            if ((g_00946f58_tbl[faction * 0x59c] & 0x80) == 0) {
                msgA = reinterpret_cast<const char *>(g_00689cc8);
                msgB = reinterpret_cast<const char *>(g_00689cd8);
            } else {
                msgA = reinterpret_cast<const char *>(g_00689cac);
                msgB = reinterpret_cast<const char *>(g_00689cb8);
            }
            popb(msgB, 0x40, -1, msgA, 0);
            base = reinterpret_cast<char *>(*g_0090ea30);
        }
    }

    if (*reinterpret_cast<int *>(base + 0x11c) > *reinterpret_cast<int *>(base + 0x118)) {
        int exempt = 0;
        int otherFaction = *g_009a658c;
        if (otherFaction >= 0 &&
            g_0097d044_tbl[otherFaction * 0x134] == base[4]) {
            exempt = 1;
        }
        if (!exempt && base[8] == 0) {
            drone_riot();
            return;
        }
    }

    if ((base[0x30] & 2) != 0) {
        base[0x30] = base[0x30] & 0xfd;
        base = reinterpret_cast<char *>(*g_0090ea30);
        if ((base[0x30] & 4) == 0) {
            faction = static_cast<unsigned char>(base[4]);
            int flags38 = *reinterpret_cast<int *>(base + 0x38);
            int hasFlag40 = (flags38 & 0x40) != 0;
            int hasSign = (flags38 & 0x80000000) != 0;
            const char *msgA;
            const char *msgB;
            int val;
            if ((g_00946f58_tbl[faction * 0x59c] & 0x80) != 0) {
                if (!hasFlag40 || !hasSign) {
                    if (!hasFlag40 || !hasSign) {
                        msgA = reinterpret_cast<const char *>(g_00689d04);
                        val = 0x10;
                        msgB = reinterpret_cast<const char *>(g_00689d10);
                    } else {
                        msgA = reinterpret_cast<const char *>(g_00689d04);
                        val = 0;
                        msgB = reinterpret_cast<const char *>(g_00689d10);
                    }
                } else {
                    msgA = reinterpret_cast<const char *>(g_00689ce8);
                    val = 0;
                    msgB = reinterpret_cast<const char *>(g_00689cf4);
                }
            } else if (!hasFlag40 || !hasSign) {
                if (!hasFlag40 || !hasSign) {
                    msgA = reinterpret_cast<const char *>(g_00689d40);
                    val = 0x10;
                    msgB = reinterpret_cast<const char *>(g_00689d50);
                } else {
                    msgA = reinterpret_cast<const char *>(g_00689d40);
                    val = 0;
                    msgB = reinterpret_cast<const char *>(g_00689d50);
                }
            } else {
                msgA = reinterpret_cast<const char *>(g_00689d20);
                val = 0;
                msgB = reinterpret_cast<const char *>(g_00689d30);
            }
            popb(msgB, 0x33, val, msgA, 0);
            base = reinterpret_cast<char *>(*g_0090ea30);
        }

        faction = static_cast<unsigned char>(base[4]);
        int mask = 1 << faction;
        if ((*reinterpret_cast<unsigned char *>(g_009a64e8) & mask) == 0) {
            base_reset(0, *g_00689370);
            base = reinterpret_cast<char *>(*g_0090ea30);
        }
        int px = *reinterpret_cast<short *>(base);
        int py = *reinterpret_cast<short *>(base + 2);
        draw_tile(px, py, 2);
    }
}
