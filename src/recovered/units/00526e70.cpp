// ORIGINAL: 0x00526E70 ?production_phase@@YAXH@Z 0x00526E70-0x00527285 FILE
// size      1045 bytes
// prototype void (__cdecl ?production_phase@@YAXH@Z)(int factionID)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0048C0A0 0x004F79C0 0x005BAE60 0x005BE530 0x005FCB20 0x00626250
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00526E70
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00526e70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?production_phase@@YAXH@Z  at 0x00526E70  (1045 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef char int8;
typedef unsigned char uint8;
typedef signed char int8_t;
typedef unsigned char uint8_t;

struct factionID;

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl base_upkeep(int baseID);
int __cdecl popp(int8*, int8*, int, int8*, int (__cdecl *)());
int __cdecl tech_advance(int factionID);
void __cdecl do_all_non_input();
void __cdecl log_say(int8*, int8*, int, int, int);
void __cdecl tech_effects(int factionID);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068b9d8 = (int *)0x0068B9D8;
static int *const g_0068b9e8 = (int *)0x0068B9E8;
static int *const g_0068b9f4 = (int *)0x0068B9F4;
static int *const g_0068ba08 = (int *)0x0068BA08;
static int *const g_0068ba18 = (int *)0x0068BA18;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a958 = (int *)0x0093A958;
static int *const g_00945f40 = (int *)0x00945F40;
static int *const g_0096eccc = (int *)0x0096ECCC;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_0097d260 = (int *)0x0097D260;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64c4 = (int *)0x009A64C4;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a64e9 = (int *)0x009A64E9;


// Per-faction record arrays, stride 0x833 ints (0x20CC bytes) indexed by faction id.
extern int g_0096cdd8[];
extern int g_0096ccdc[];
extern int g_0096cce0[];
extern int g_0096cce8[];
extern int g_0096ccf0[];
extern int g_0096ccec[];
extern int g_0096caa0[];
extern int g_0096cc00[];
extern unsigned int g_0096c9e0[];
extern int g_0096d1b0[];
extern int g_0096d090[];
extern int g_0096d070[];
extern int g_0096d1d4[];
extern int g_0096d1d8[];
extern int g_0096d1dc[];
extern int g_0096d1e0[];
extern int g_0096d1e4[];

// Faction x faction commerce/coefficient table, stride 0x833 ints, indexed
// by [otherFaction + thisFaction*0x833].
extern int g_0096caa4[];
extern int g_0096cac4[];
extern int g_0096c9f8[];

// Per-base owner id, one byte per base (0x134-byte base records).
extern unsigned char g_x97d044[];

// Per-faction rules record, stride 0x167 ints (0x59C bytes).
extern unsigned int g_00946f58[];
extern int g_00946f3c[];
extern int g_00946f24[];
extern char g_00946a84[];

void __cdecl production_phase(int a1) {
    int rec = a1 * 0x833;

    g_0096cdd8[rec] = 0;
    g_0096ccdc[rec] = 0;
    g_0096cce0[rec] = 0;
    g_0096cce8[rec] = 0;

    tech_effects(a1);

    int *slot = g_0096eccc;
    for (int j = 1; slot < (int *)0x97d260; slot += 0x833, ++j) {
        if (j != a1 &&
            (*g_009a64e9 & (1 << (j & 0x1f))) != 0 &&
            (*g_009a64e9 & (1 << (a1 & 0x1f))) != 0) {
            int idx = j + a1 * 0x833;
            if (g_0096caa4[idx] != 0 && slot[-0x58] == 0) {
                if ((g_0096c9f8[idx] & 0x10) == 0) {
                    int give = g_0096cac4[idx];
                    if (g_0096cc00[rec] <= g_0096cac4[idx])
                        give = g_0096cc00[rec];
                    *slot = *slot + give;
                    g_0096caa4[idx] = g_0096caa4[idx] - give;
                    g_0096cc00[rec] = g_0096cc00[rec] - give;
                    if (give < g_0096cac4[idx]) {
                        g_0096caa4[idx] = g_0096caa4[idx] + (g_0096cac4[idx] - give);
                    }
                } else {
                    g_0096caa4[idx] = g_0096cac4[idx] + g_0096caa4[idx];
                }
            }
        }
    }

    g_0096c9e0[rec] = g_0096c9e0[rec] & 0xffffffdfu;
    *g_0093a958 = g_0096cc00[rec];

    int *p9 = &g_0096d1b0[rec];
    for (int i = 9; i != 0; --i) {
        *p9 = 0;
        ++p9;
    }

    int *blk = &g_0096d090[rec];
    int *one = &g_0096d070[rec];
    int outer = 8;
    do {
        *one = 0;
        int *inner = blk;
        for (int i = 9; i != 0; --i) {
            *inner = 0;
            ++inner;
        }
        ++one;
        blk += 9;
        --outer;
    } while (outer != 0);

    g_0096d1d4[rec] = 0;
    g_0096d1d8[rec] = 0;
    g_0096d1dc[rec] = 0;
    g_0096d1e0[rec] = 0;
    g_0096d1e4[rec] = 0;

    int baseCount = *g_009a64cc;
    if (baseCount > 0) {
        int b = 0;
        unsigned char *owner = g_x97d044;
        do {
            if (*owner == a1) {
                if (base_upkeep(b) != 0) {
                    --b;
                    owner -= 0x134;
                }
                do_all_non_input();
            }
            ++b;
            owner += 0x134;
        } while (b < *g_009a64cc);
    }

    if ((*g_009a64e8 & (1 << (a1 & 0x1f))) == 0 && *g_009a64c4 > 3) {
        int trade = g_0096cce0[rec];
        if (trade != 0) {
            int cut, roundedCut;
            if (*g_009a64c4 == 4) {
                cut = trade / 3;
                g_0096cc00[rec] = g_0096cc00[rec] + cut;
                trade = g_0096cce0[rec];
                roundedCut = trade / 3 + (trade >> 0x1f);
            } else {
                cut = (trade * 2) / 3;
                g_0096cc00[rec] = g_0096cc00[rec] + cut;
                trade = g_0096cce0[rec];
                roundedCut = (trade * 2) / 3 + ((trade * 2) >> 0x1f);
            }
            g_0096cce0[rec] = trade - (roundedCut - (roundedCut >> 0x1f));
        }
    }

    int rules = a1 * 0x167;
    if ((g_00946f58[rules] & 0x4000) != 0) {
        if (g_00946f3c[rules] == 0) {
            if (*g_009a64cc > 0) {
                unsigned char *owner = g_x97d044;
                int cnt = *g_009a64cc;
                do {
                    if (*owner == a1) {
                        g_0096cc00[rec] = g_0096cc00[rec] + 1;
                        g_0096ccdc[rec] = g_0096ccdc[rec] + 1;
                    }
                    owner += 0x134;
                    --cnt;
                } while (cnt != 0);
            }
        } else {
            g_0096cc00[rec] = (g_00946f3c[rules] * g_0096cc00[rec]) / 100 + g_0096cc00[rec];
        }
    }

    g_0096ccf0[rec] = g_0096ccdc[rec];
    g_0096ccec[rec] = (g_0096cce8[rec] - g_0096cce0[rec]) + g_0096ccdc[rec];

    if (*g_009a64d4 == 1) {
        *g_00945f40 = 1;
        int n = 0;
        if (g_00946f24[rules] > 0) {
            do {
                log_say((int8 *)0x68b9d8, g_00946a84 + rules * 4, a1, 0, 0);
                tech_advance(a1);
                ++n;
            } while (n < g_00946f24[rules]);
        }
        *g_00945f40 = 0;
    }

    if ((((*(unsigned char *)g_009a64c0 & 1) == 0) ||
         ((*(unsigned char *)g_009a64c0 & 8) != 0)) &&
        g_0096caa0[rec] != 0) {
        int remain = g_0096caa0[rec] - 1;
        g_0096caa0[rec] = remain;
        if (remain == 0 && a1 == *g_00939284) {
            if ((g_00946f58[rules] & 0x80) != 0) {
                popp(reinterpret_cast<int8 *>(*g_00691b0c), (int8 *)0x68b9f4, 0, (int8 *)0x68b9e8, 0);
                return;
            }
            popp(reinterpret_cast<int8 *>(*g_00691b0c), (int8 *)0x68ba18, 0, (int8 *)0x68ba08, 0);
        }
    }
}
