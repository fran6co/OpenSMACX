// ORIGINAL: 0x0055EEE0 ?atrocity@@YAXHHHH@Z 0x0055EEE0-0x0055F44F FILE
// size      1391 bytes
// prototype void (__cdecl ?atrocity@@YAXHHHH@Z)(int factionID1, int factionID2, int, int)
// callers   4   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A890 0x005399A0 0x00539C00 0x0053A030 0x0055B870 0x0055F450 0x005BF5D0 0x00625E30 0x00625EC0 0x00645470 0x0064601D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0055EEE0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0055eee0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?atrocity@@YAXHHHH@Z  at 0x0055EEE0  (1391 bytes)
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
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
extern "C" char *strcat(char *, const char *);
extern "C" int rand();
int __cdecl X_pops(const char *, Sprite *, int (__cdecl *)());
int __cdecl great_satan(int, int);
int __cdecl parse_num(int, int);
int __cdecl parse_says(int, char *, int, int);
void __cdecl cause_friction(int, int, int);
void __cdecl diplomacy_caption(int, int);
void __cdecl major_atrocity(int, int);
void __cdecl treaty_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00400000 = (int *)0x00400000;
static int *const g_006846dc = (int *)0x006846DC;
static int *const g_0068e73c = (int *)0x0068E73C;
static int *const g_0068e75c = (int *)0x0068E75C;
static int *const g_0068e76c = (int *)0x0068E76C;
static int *const g_0068e778 = (int *)0x0068E778;
static int *const g_0068e77c = (int *)0x0068E77C;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009474f4 = (int *)0x009474F4;
static int *const g_00949c38 = (int *)0x00949C38;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096cbb4 = (int *)0x0096CBB4;
static int *const g_0096eac4 = (int *)0x0096EAC4;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c4 = (int *)0x009A64C4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a6638 = (int *)0x009A6638;
static int *const g_009a663c = (int *)0x009A663C;
static int *const g_009a6800 = (int *)0x009A6800;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

extern uint8_t g_0096c9f8x[];
extern uint8_t g_0096ca18x[];
extern uint8_t g_0096eac4x[];
extern uint8_t g_0096cbb4x[];
extern uint8_t g_0096c9e0x[];
extern uint8_t g_0096c9e8x[];
extern uint8_t g_0096caa0x[];
extern uint8_t g_0096cb04x[];
extern uint8_t g_00946a50x[];
extern uint8_t g_00946a84x[];
extern uint8_t g_00946a9cx[];
extern uint8_t g_00946f58x[];
extern uint8_t g_009474f4x[];

static unsigned int &U32(uint8_t *base, int byteOffset) {
    return *reinterpret_cast<unsigned int *>(base + byteOffset);
}

void __cdecl atrocity(int a1, int a2, int a3, int a4) {
    int pairIdx = (a1 + a2 * 0x833) * 4;
    unsigned int wasAllied = U32(g_0096c9f8x, pairIdx) & 0x40000;
    U32(g_0096c9f8x, pairIdx) |= 0x40020;
    int lastAtrocity = *g_009a663c;
    U32(g_0096ca18x, pairIdx) |= 4;
    if (lastAtrocity < *g_009a6638) {
        return;
    }
    if (*g_009a6800 > 0) {
        if (a1 != *g_00939284) {
            return;
        }
        NetMsg *msg = reinterpret_cast<NetMsg *>(g_00805338);
        msg->pop(reinterpret_cast<const char *>(g_0068e73c), 5000, 0, 0 /* NULL */);
        return;
    }
    if (*g_009a6488 != 0 && a4 == 0) {
        if ((g_00946f58x[a1 * 0x59c] & 0x80) != 0) return;
        if ((g_00946f58x[a2 * 0x59c] & 0x80) != 0) return;
    }

    uint8_t *factionRec = g_0096eac4x + a1;
    int otherId = 1;
    Sprite **spriteTable = reinterpret_cast<Sprite **>(g_006846dc);
    uint8_t *checkPtr = g_009474f4x;

    do {
        if (otherId != a1 && otherId != a2 && (U32(factionRec, 0) & 8) != 0 &&
            (*g_009a6488 == 0 || a4 == 0 ||
             (g_00946f58x[a1 * 0x59c] & 0x80) != 0 || (checkPtr[0] & 0x80) == 0)) {
            if ((U32(factionRec, 0) & 0x200000) != 0) {
                cause_friction(otherId, a1, 5);
                unsigned int flags = *g_009a64e8 & 0xff;
                bool okToAttack = ((flags & (1 << (otherId & 0x1f))) == 0) &&
                                   ((flags & (1 << (a1 & 0x1f))) != 0) &&
                                   ((U32(g_0096c9f8x, otherId * 4 + a1 * 0x20cc) & 8) != 0) &&
                                   ((U32(g_0096c9f8x, otherId * 4 + a1 * 0x20cc) & 0x11) == 0) &&
                                   (wasAllied == 0);
                if (okToAttack) {
                    bool doIt;
                    if ((U32(factionRec, 0) & 0x20) == 0) {
                        int gs = great_satan(a1, 0);
                        if (gs == 0) {
                            int denom = 5;
                            if ((*reinterpret_cast<uint8_t *>(g_009a649c) & 0x40) == 0) {
                                denom = *g_009a64c4;
                            }
                            denom = (8 - denom) * 2;
                            int r;
                            if (denom - 1 < 1) {
                                r = 0;
                            } else {
                                r = rand() % denom;
                            }
                            doIt = (r == 0);
                        } else {
                            int rv = rand();
                            int uv = rv & 0x8000000f;
                            if (uv < 0) {
                                uv = ((uv - 1) | 0xfffffff0) + 1;
                            }
                            doIt = (uv - static_cast<int>(U32(g_0096cbb4x, a1 * 0x20cc))) < 9;
                        }
                    } else {
                        doIt = true;
                    }
                    if (doIt) {
                        if (a1 == *g_00939284) {
                            *g_009bbff0 = 0;
                            *g_009bbfec = U32(g_00946a50x, a1 * 0x59c);
                            parse_says(0, reinterpret_cast<char *>(g_00946a9cx + a1 * 0x59c), -1, -1);
                            *g_009bbfec = U32(g_00946a50x, a1 * 0x59c);
                            *g_009bbff0 = 0;
                            parse_says(1, reinterpret_cast<char *>(g_00946a84x + a1 * 0x59c), -1, -1);
                            diplomacy_caption(a1, otherId);
                            X_pops(reinterpret_cast<const char *>(g_0068e75c), *spriteTable, 0);
                        }
                        treaty_on(a1, otherId, 0x10);
                        U32(factionRec, 0) |= 0x40;
                        factionRec[0x80] = 0x32;
                    }
                }
            }
            if ((U32(factionRec, 0) & 0x10) == 0) {
                treaty_on(otherId, a1, 0x400000);
            }
        }
        checkPtr += 0x59c;
        ++otherId;
        ++spriteTable;
        factionRec += 0x20cc;
    } while (checkPtr < g_009474f4x + (reinterpret_cast<char *>(g_00949c38) - reinterpret_cast<char *>(g_009474f4x)));

    if (a3 != 0 || U32(g_0096cbb4x, a1 * 0x20cc) == 0) {
        U32(g_0096cbb4x, a1 * 0x20cc) += 1;
        if ((U32(g_0096c9f8x, a2 * 4 + a1 * 0x20cc) & 0x40000) == 0) {
            U32(g_0096cb04x, a1 * 0x20cc) += 1;
        }
    }
    unsigned int atrocities = U32(g_0096cbb4x, a1 * 0x20cc);
    if (static_cast<int>((8 - g_0096c9e8x[a1 * 0x833]) * 4) < static_cast<int>(atrocities)) {
        major_atrocity(a1, a2);
        return;
    }
    unsigned int flags = *g_009a64e8 & 0xff;
    if (((flags & (1 << (a1 & 0x1f))) != 0) && ((flags & (1 << (otherId & 0x1f))) == 0) && wasAllied == 0 &&
        atrocities > 4) {
        U32(g_0096c9e0x, a2 * 0x20cc) |= 0x1000000;
    }
    *g_009bbff0 = 0;
    *g_009bbfec = U32(g_00946a50x, a1 * 0x59c);
    parse_says(0, reinterpret_cast<char *>(g_00946a9cx + a1 * 0x59c), -1, -1);
    *g_009bbfec = U32(g_00946a50x, a1 * 0x59c);
    *g_009bbff0 = 0;
    parse_says(1, reinterpret_cast<char *>(g_00946a84x + a1 * 0x59c), -1, -1);
    *g_009bbff0 = 0;
    *g_009bbfec = U32(g_00946a50x, a2 * 0x59c);
    parse_says(2, reinterpret_cast<char *>(g_00946a9cx + a2 * 0x59c), -1, -1);
    *g_009bbfec = U32(g_00946a50x, a2 * 0x59c);
    *g_009bbff0 = 0;
    parse_says(3, reinterpret_cast<char *>(g_00946a84x + a2 * 0x59c), -1, -1);
    parse_num(0, static_cast<int>(atrocities) * 10);

    char *buf = reinterpret_cast<char *>(g_009b86a0);
    buf[0] = 0;
    strcat(buf, reinterpret_cast<char *>(g_0068e76c));
    if (a1 == *g_00939284) {
        strcat(buf, reinterpret_cast<char *>(g_0068e778));
    } else if (a2 == a1) {
        strcat(buf, reinterpret_cast<char *>(g_0068e77c));
    }
    if ((g_00946f58x[a1 * 0x59c] & 0x80) == 0) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(buf, 5000, 0, 0);
    }
    U32(g_0096caa0x, a1 * 0x20cc) += atrocities * 10;
}
