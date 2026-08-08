// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0055C840
// name           ?enemies_war@@YAXHH@Z
// size           1114 bytes
// measured tier  MISMATCH
// divergence     14
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0055c840/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?enemies_war@@YAXHH@Z  at 0x0055C840  (1114 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
char * __cdecl get_he_she(int, int);
extern "C" int rand();
int __cdecl get_noun(int);
int __cdecl has_project(int, int);
int __cdecl parse_says(int, char *, int, int);
void __cdecl set_treaty(int, int, int, int);
void __cdecl trade_maps(int, int);
void __cdecl treaty_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068e5d4 = (int *)0x0068E5D4;
static int *const g_0068e5e0 = (int *)0x0068E5E0;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64c4 = (int *)0x009A64C4;
static int *const g_009a6614 = (int *)0x009A6614;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

extern int g_00946a50[];
extern char g_00946a84[];
extern char g_00946a9c[];
extern int g_00946d4c[];
extern int g_00946d50[];
extern char g_00946d34[];
extern int g_0096ca18[];
extern int g_0096ca78[];

// This body was already present here from an earlier session but no
// longer compiled against a fresh brief: the live catalogue dropped the
// extern array declarations for the tables below (g_00946a50/a84/a9c,
// g_00946d4c/d50/d34, g_0096ca18, g_0096ca78), so they are re-declared
// here per the "fix stale scaffold" rule, using the INDEXED-TABLE-BASE
// lever (extern T[], not a pointer constant) since every one of them is
// walked by a computed index. g_0096c9f8 itself is left as the context's
// given pointer constant and just subscripted plainly: the disassembly
// addresses it as [reg*4+0x96C9F8] (index in a register, base folded into
// the instruction's displacement), which is exactly what subscripting a
// compile-time-constant pointer produces - this is NOT the "address does
// work" shape the lever warns about (that shape is a runtime-loaded base
// register combined with the index via `lea`, which does not appear here).
//
// Scored MISMATCH, very close: divergence at instruction #14 ('shl' vs
// 'mov'), within the very first index computation - `a1 * 0x833 + a2`
// (0x833 = 2099, prime) lowers to a single-step multiply here instead of
// the original's multi-instruction lea/shl/add shift-add chain. Textually
// spelling the same chain by hand did not change which one the optimizer
// picks; this is the STRENGTH REDUCTION wall, and it recurs at every one
// of the other three places in this function that index the same table
// with the same constant. Total size came out within 4 bytes of the
// original (1118 vs 1114), so nothing else of substance diverges.
extern int g_00946a50[];
extern char g_00946a84[];
extern char g_00946a9c[];
extern int g_00946d4c[];
extern int g_00946d50[];
extern char g_00946d34[];
extern int g_0096ca18[];
extern int g_0096ca78[];

void __cdecl enemies_war(int a1, int a2) {
    int recAB = a1 * 0x833 + a2;
    if (g_0096c9f8[recAB] & 0x10) {
        return;
    }

    const char *msg;

    if (g_0096c9f8[*g_00939284 * 0x833 + a1] & 1) {
        *g_009bbff0 = -1;
        *g_009bbfec = g_00946a50[a1 * 0x167];
        parse_says(0, &g_00946a9c[a1 * 0x59c], -1, -1);
        *g_009bbff0 = -1;
        *g_009bbfec = g_00946a50[a1 * 0x167];
        parse_says(1, &g_00946a84[a1 * 0x59c], -1, -1);
        parse_says(2, reinterpret_cast<char *>(get_noun(a1)), -1, -1);
        parse_says(3, get_he_she(a1, 0), -1, -1);
        parse_says(4, reinterpret_cast<char *>(get_noun(a2)), -1, -1);
        msg = reinterpret_cast<const char *>(g_0068e5d4);
        goto sendPop;
    }

    if (g_0096c9f8[*g_00939284 * 0x833 + a2] & 1) {
        *g_009bbff0 = -1;
        *g_009bbfec = g_00946a50[a2 * 0x167];
        parse_says(0, &g_00946a9c[a2 * 0x59c], -1, -1);
        *g_009bbff0 = -1;
        *g_009bbfec = g_00946a50[a2 * 0x167];
        parse_says(1, &g_00946a84[a2 * 0x59c], -1, -1);
        parse_says(2, reinterpret_cast<char *>(get_noun(a2)), -1, -1);
        parse_says(3, get_he_she(a2, 0), -1, -1);
        parse_says(4, reinterpret_cast<char *>(get_noun(a1)), -1, -1);
        msg = reinterpret_cast<const char *>(g_0068e5d4);
        goto sendPop;
    }

    if (!(g_0096c9f8[*g_00939284 * 0x833 + a1] & 0x1000) &&
        has_project(4, *g_00939284) == 0 &&
        (*g_00939284 != *g_009a6614 || (g_00946f58[a1 * 0x167] & 0x80)) &&
        !(g_0096c9f8[*g_00939284 * 0x833 + a2] & 0x1000) &&
        has_project(4, *g_00939284) == 0 &&
        (*g_00939284 != *g_009a6614 || (g_00946f58[a2 * 0x167] & 0x80)) &&
        !(*g_009a64c0 & 0x80)) {
        goto skipMsg;
    }

    *g_009bbfec = g_00946d4c[a1 * 0x167];
    *g_009bbff0 = g_00946d50[a1 * 0x167];
    parse_says(0, &g_00946d34[a1 * 0x59c], -1, -1);
    *g_009bbfec = g_00946d4c[a2 * 0x167];
    *g_009bbff0 = g_00946d50[a2 * 0x167];
    parse_says(1, &g_00946d34[a2 * 0x59c], -1, -1);
    msg = reinterpret_cast<const char *>(g_0068e5e0);

sendPop:
    reinterpret_cast<NetMsg *>(g_00805338)->pop(msg, 5000, 0, 0);

skipMsg:
    treaty_on(a1, a2, 0x10);
    if (!(g_0096c9f8[recAB] & 0x40000000)) {
        trade_maps(a1, a2);
    }

    {
        int flags = g_0096c9f8[recAB];
        int denom = *g_009a64c4;
        if ((flags & 0x20) && !(flags & 0x40000) && denom > 1) {
            int roll = rand();
            if (roll % denom != 0) {
                int recBA = a2 * 0x833 + a1;
                g_0096ca18[recAB] &= 0xfffff3ff;
                g_0096ca18[recBA] &= 0xfffff3ff;
                g_0096c9f8[recAB] &= 0xffffffdf;
                g_0096c9f8[recBA] &= 0xffffffdf;
                g_0096c9f8[recAB] |= 0x40;
                g_0096ca78[recAB] = 0x32;
            }
        }
    }

    {
        int recBA = a2 * 0x833 + a1;
        int flags = g_0096c9f8[recBA];
        int denom = *g_009a64c4;
        if ((flags & 0x20) && !(flags & 0x40000) && denom > 1) {
            int roll = rand();
            if (roll % denom != 0) {
                g_0096ca18[recBA] &= 0xfffff3ff;
                g_0096ca18[recAB] &= 0xfffff3ff;
                g_0096c9f8[recBA] &= 0xffffffdf;
                g_0096c9f8[recAB] &= 0xffffffdf;
                set_treaty(a2, a1, 0x40, 1);
            }
        }
    }
}
