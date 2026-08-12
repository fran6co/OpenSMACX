// ORIGINAL: 0x00558C60 FILE
// name      ?commlink_attempt@@YAXH@Z
// size      852 bytes
// spans     0x00558C60-0x00558FB4
// prototype 
// callers   3   call targets   13
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00444D90 0x0047A890 0x005304A0 0x005315C0 0x0054FFD0 0x005589E0 0x0055B870 0x00592EE0 0x005B9F20 0x005BF7D0 0x00625E30 0x00625EC0 0x006262F0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00558C60
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00558c60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?commlink_attempt@@YAXH@Z  at 0x00558C60  (852 bytes)
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
    int open_channel(int, int);
    void await_diplo(int);
};
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
bool __cdecl has_tech(int, int);
int __cdecl X_pops(const char *, int, Sprite *, int (__cdecl *)());
int __cdecl commlink_attempter(int, int);
int __cdecl parse_num(int, int);
int __cdecl parse_says(int, char *, int, int);
void __cdecl communicate(int, int, int);
void __cdecl diplo(int, int);
void __cdecl log_say(char *, int, int, int);
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl treaty_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_006846d8 = (int *)0x006846D8;
static int *const g_0068d444 = (int *)0x0068D444;
static int *const g_0068e4d4 = (int *)0x0068E4D4;
static int *const g_0068e4e0 = (int *)0x0068E4E0;
static int *const g_0068e4f0 = (int *)0x0068E4F0;
static int *const g_0068e4fc = (int *)0x0068E4FC;
static int *const g_0068e510 = (int *)0x0068E510;
static int *const g_0068e520 = (int *)0x0068E520;
static int *const g_0068e534 = (int *)0x0068E534;
static int *const g_0068e540 = (int *)0x0068E540;
static int *const g_0068e558 = (int *)0x0068E558;
static int *const g_0073b790 = (int *)0x0073B790;
static int *const g_007492cc = (int *)0x007492CC;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a64e9 = (int *)0x009A64E9;
static int *const g_009a6800 = (int *)0x009A6800;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// Register-indexed table bases - the address itself does work (added to
// a runtime byte offset), so these need real extern linkage rather than
// the folded pointer constants above.
extern char g_00946f58_tbl[];
extern char g_00946a50_tbl[];
extern char g_00946a84_tbl[];
extern char g_00946a9c_tbl[];
extern int g_006846d8_tbl[];

void __cdecl commlink_attempt(int a1) {
    int otherFaction = *g_00939284;
    if (otherFaction == a1) {
        return;
    }

    if ((*reinterpret_cast<unsigned char *>(g_009a64e9) &
         static_cast<unsigned char>(1 << a1)) == 0) {
        return;
    }

    if (*g_009a6800 > 0) {
        reinterpret_cast<NetMsg *>(g_00805338)
            ->pop(reinterpret_cast<const char *>(g_0068e4d4), 5000, 0,
                  static_cast<const char *>(0));
        return;
    }

    if ((*reinterpret_cast<int *>(g_00946f58_tbl + otherFaction * 0x59c) & 0x80) !=
            0 &&
        (*reinterpret_cast<unsigned char *>(g_00946f58_tbl + a1 * 0x59c) & 0x80) !=
            0) {
        int savedId = *reinterpret_cast<int *>(g_00946a50_tbl + otherFaction * 0x59c);
        char *nameAddr = g_00946a9c_tbl + otherFaction * 0x59c;
        *g_009bbfec = savedId;
        *g_009bbff0 = 0;
        parse_says(1, nameAddr, -1, -1);
        int savedId2 = *reinterpret_cast<int *>(g_00946a50_tbl + otherFaction * 0x59c);
        char *nameAddr2 = g_00946a84_tbl + otherFaction * 0x59c;
        *g_009bbfec = savedId2;
        *g_009bbff0 = 0;
        parse_says(3, nameAddr2, -1, -1);
        X_pops(reinterpret_cast<const char *>(g_0068e4e0), 0x100000,
               reinterpret_cast<Sprite *>(g_006846d8_tbl[a1]),
               reinterpret_cast<int (__cdecl *)()>(g_005398e0));
        return;
    }

    if ((*reinterpret_cast<int *>(g_00946f58_tbl + otherFaction * 0x59c) & 0x80) !=
        0) {
        if (!has_tech(0x4e, a1) && !has_tech(4, otherFaction)) {
            reinterpret_cast<NetMsg *>(g_00805338)
                ->pop(reinterpret_cast<const char *>(g_0068e4fc), -5000, 0,
                      reinterpret_cast<const char *>(g_0068e4f0));
            return;
        }
    }

    if ((*reinterpret_cast<unsigned char *>(g_00946f58_tbl + a1 * 0x59c) & 0x80) !=
        0) {
        if (!has_tech(0x4e, otherFaction) && !has_tech(4, a1)) {
            reinterpret_cast<NetMsg *>(g_00805338)
                ->pop(reinterpret_cast<const char *>(g_0068e520), -5000, 0,
                      reinterpret_cast<const char *>(g_0068e510));
            return;
        }
    }

    if (commlink_attempter(otherFaction, a1) != 0) {
        return;
    }

    if (*g_0093f660 != 0) {
        if ((static_cast<unsigned char>(1 << a1) &
             *reinterpret_cast<unsigned char *>(g_009a64e8)) != 0) {
            message_data(0x1501, a1, otherFaction, a1, 0, 0);
            *g_007492cc = 1;
            *g_0073b790 = a1;
            int savedId = *reinterpret_cast<int *>(g_00946a50_tbl + a1 * 0x59c);
            char *nameAddr = g_00946a9c_tbl + a1 * 0x59c;
            *g_009bbfec = savedId;
            *g_009bbff0 = 0;
            parse_says(0, nameAddr, -1, -1);
            int savedId2 = *reinterpret_cast<int *>(g_00946a50_tbl + a1 * 0x59c);
            char *nameAddr2 = g_00946a84_tbl + a1 * 0x59c;
            *g_009bbfec = savedId2;
            *g_009bbff0 = 0;
            parse_says(1, nameAddr2, -1, -1);
            parse_num(0, a1);
            reinterpret_cast<NetMsg *>(g_00805338)
                ->pop(reinterpret_cast<const char *>(g_0068e534), 5000, 0,
                      static_cast<const char *>(0));
            return;
        }

        log_say(reinterpret_cast<char *>(g_0068e540), otherFaction, a1, 0);
        if (reinterpret_cast<NetDaemon *>(g_0093cd90)
                ->open_channel(otherFaction, a1) != 0) {
            int savedId = *reinterpret_cast<int *>(g_00946a50_tbl + a1 * 0x59c);
            char *nameAddr = g_00946a9c_tbl + a1 * 0x59c;
            *g_009bbfec = savedId;
            *g_009bbff0 = 0;
            parse_says(0, nameAddr, -1, -1);
            int savedId2 = *reinterpret_cast<int *>(g_00946a50_tbl + a1 * 0x59c);
            char *nameAddr2 = g_00946a84_tbl + a1 * 0x59c;
            *g_009bbfec = savedId2;
            *g_009bbff0 = 0;
            parse_says(1, nameAddr2, -1, -1);
            reinterpret_cast<NetMsg *>(g_00805338)
                ->pop(reinterpret_cast<const char *>(g_0068e558), 5000, 0,
                      static_cast<const char *>(0));
            return;
        }
    }

    if (*g_0093a95c != 0 &&
        (static_cast<unsigned char>(1 << a1) &
         *reinterpret_cast<unsigned char *>(g_009a64e8)) != 0) {
        diplo(otherFaction, a1);
        return;
    }

    communicate(otherFaction, a1, 1);
    if (*g_0093f660 == 0) {
        treaty_on(otherFaction, a1, 0x88000008);
        return;
    }

    log_say(reinterpret_cast<char *>(g_0068d444), otherFaction, a1, 0x88000008);
    message_data(0x2441, 0, otherFaction, a1, 0x88000008, 0);
    reinterpret_cast<NetDaemon *>(g_0093cd90)->await_diplo(0x441);
}
