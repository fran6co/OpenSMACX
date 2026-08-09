// ORIGINAL: 0x005BE940 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005BE940
// name           ?tech_research@@YAXHH@Z
// size           560 bytes
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005be940/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?tech_research@@YAXHH@Z  at 0x005BE940  (560 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl climactic_battle();
int __cdecl tech_advance(int);
int __cdecl tech_rate(int);
int __cdecl tech_selection(int);
void __cdecl log_say(char *, char *, int, int, int);
void __cdecl mon_tech_discovered(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00691848 = (int *)0x00691848;
static int *const g_0069185c = (int *)0x0069185C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00945f48 = (int *)0x00945F48;
static int *const g_00946a84 = (int *)0x00946A84;
static int *const g_009a51ac = (int *)0x009A51AC;
static int *const g_009a5e9c = (int *)0x009A5E9C;
static int *const g_009a64e8 = (int *)0x009A64E8;

// Indexed table bases the emitter did not catch (register-scaled access, not
// a plain load): a two-int-wide record (fields at +0 and +4) with an 0x2C
// stride at 0x94f37c/0x94f380, a one-byte-per-index flag table at 0x9a6670,
// and the per-faction record (stride 0x20CC) holding the four fields this
// body reads/writes.
extern int g_0094f37c[];
extern int g_0094f380[];
extern unsigned char g_009a6670[];
extern int g_0096cda8[];
extern int g_0096cdac[];
extern int g_0096cdb0[];
extern unsigned char g_0096c9e0[];

void __cdecl tech_research(int a1, int a2) {
    if (a2 <= 0)
        return;

    int mask = 1 << a1;
    int skipDoubling;

    if ((*reinterpret_cast<unsigned char *>(g_009a64e8) & mask) != 0
        || climactic_battle() == 0) {
        skipDoubling = 1;
    } else {
        skipDoubling = 0;
        if (a1 > 0) {
            int idx = *g_009a51ac;
            if (idx != -1) {
                if (idx < 0 || idx >= 0x58)
                    goto doDouble;
                if (g_0094f37c[idx * 11] < -1)
                    goto doDouble;
                if (!(g_0094f380[idx * 11] >= -1 || g_0094f37c[idx * 11] == -1))
                    goto doDouble;
                if ((g_009a6670[idx] & mask) == 0)
                    goto doDouble;
            }
            idx = *g_009a5e9c;
            if (idx == -1) {
                skipDoubling = 1;
                goto afterDouble;
            }
            if (idx < 0 || idx >= 0x58)
                goto doDouble;
            if (g_0094f37c[idx * 11] < -1)
                goto doDouble;
            if (!(g_0094f380[idx * 11] >= -1 || g_0094f37c[idx * 11] == -1))
                goto doDouble;
            if ((g_009a6670[idx] & mask) != 0) {
                skipDoubling = 1;
                goto afterDouble;
            }
        }
    doDouble:
        a2 = a2 * 2;
    }
afterDouble:
    (void)skipDoubling;

    int recOffset = a1 * 0x20CC;
    int *cda8 = reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096cda8) + recOffset);
    int *cdac = reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096cdac) + recOffset);
    int *cdb0 = reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096cdb0) + recOffset);
    unsigned char *c9e0 = reinterpret_cast<unsigned char *>(g_0096c9e0) + recOffset;

    int globalFlag = *g_0093f660;
    *cda8 = *cda8 + a2;
    if (globalFlag == 0 && *cdac < 0) {
        goto afterResearch;
    }

    {
        int rate = tech_rate(a1);
        if (*cdac >= 0x59) {
            *cda8 = *cda8 + a2;
        }
        if (*cda8 >= rate) {
            log_say(reinterpret_cast<char *>(g_00691848),
                    reinterpret_cast<char *>(g_00946a84) + a1 * 0x59c, a1, 0, 0);
            int advance = tech_advance(a1);
            if (advance >= 0)
                mon_tech_discovered(a1, advance);
            *cda8 = 0;
            *cdb0 = -1;
            *cdb0 = tech_rate(a1);
        }
        if ((*c9e0 & 0x40) != 0) {
            log_say(reinterpret_cast<char *>(g_0069185c),
                    reinterpret_cast<char *>(g_00946a84) + a1 * 0x59c, a1, 0, 0);
            *c9e0 = *c9e0 & 0xbf;
            int advance = tech_advance(a1);
            if (advance >= 0)
                mon_tech_discovered(a1, advance);
        }
    }

afterResearch:
    if (*cda8 >= 0 && *cdac < 0 && *g_00945f48 != 1) {
        if ((*reinterpret_cast<unsigned char *>(g_009a64e8) & mask) == 0
            || a1 == *g_00939284 || *g_0093a95c == 0) {
            *cdac = tech_selection(a1);
        }
    }
}
