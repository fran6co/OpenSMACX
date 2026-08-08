// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005062B0
// name           ?promote@@YAXH@Z
// size           475 bytes
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005062b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?promote@@YAXH@Z  at 0x005062B0  (475 bytes)
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
extern "C" int rand();
int __cdecl morale_veh(int, int, int);
int __cdecl parse_say(int, int, int, int);
int __cdecl parse_says(int, char *, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068a098 = (int *)0x0068A098;
static int *const g_0068a0a8 = (int *)0x0068A0A8;
static int *const g_0068a0b0 = (int *)0x0068A0B0;
static int *const g_0068a0c0 = (int *)0x0068A0C0;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00952328 = (int *)0x00952328;
static int *const g_0095232c = (int *)0x0095232C;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009ab868 = (int *)0x009AB868;
static int *const g_009ab88d = (int *)0x009AB88D;

// Per-unit record table, stride 0x34, indexed by a1. Fields used here:
// +0x0A unit-type id (short), +0x0E team/faction byte, +0x24 morale counter byte.
extern unsigned char g_tbl1[];
// Unit-type record table, stride 0x34, indexed by unit-type id.
extern unsigned char g_tbl2[];
// Lookups keyed off single bytes pulled from g_tbl2/g_tbl1, various strides.
extern unsigned char g_94a37d[];  // stride 0x90, indexed by g_tbl2[+0x9ab88c-base]
extern signed char g_94f280[];    // stride 0x10, indexed by g_tbl2[+0x9ab88e-base]
extern unsigned char g_9ab88d[];  // stride 0x34, indexed by unit-type id
extern signed char g_94ae68[];    // stride 0x10, indexed by g_9ab88d byte
// String table for parse_says, stride 0x34, indexed by unit-type id.
extern char g_9ab868_arr[];
// Result tables for parse_say, 8-byte stride (read as int[.. *2]).
extern int g_95232c_arr[];
extern int g_952328_arr[];

void __cdecl promote(int a1) {
    unsigned char *row1 = g_tbl1 + a1 * 0x34;
    int unit_type = *reinterpret_cast<short *>(row1 + 0x0A);

    unsigned char *row2 = g_tbl2 + unit_type * 0x34;
    unsigned char class_byte = row2[0];  // 0x9AB88C

    if (g_94a37d[class_byte * 0x90] != 0) {
        return;
    }

    if (!(row2[0x9ab892 - 0x9ab88c] < 8 ||
          g_94f280[row2[0x9ab88e - 0x9ab88c] * 0x10] > 1)) {
        return;
    }
    if (morale_veh(a1, 1, 0) >= 6) {
        return;
    }
    if (row1[0x24] >= 6) {
        return;
    }

    if (morale_veh(a1, 1, 0) > 2) {
        if (rand() % 2 != 0) {
            return;
        }
    }

    int morale_result = morale_veh(a1, 1, 0);
    row1[0x24] = row1[0x24] + 1;
    unsigned char team = row1[0x0E];
    if (team != *g_00939284) {
        return;
    }
    if (!((1 << team) & *g_009a64e8)) {
        return;
    }
    if (morale_result == morale_veh(a1, 1, 0)) {
        return;
    }

    parse_says(1, g_9ab868_arr + unit_type * 0x34, -1, -1);

    if (unit_type < 0x40) {
        unsigned char armor = g_9ab88d[unit_type * 0x34];
        if (g_94ae68[armor * 0x10] < 0) {
            int r = morale_veh(a1, 1, 0);
            parse_say(0, g_95232c_arr[r * 2], -1, -1);
            reinterpret_cast<NetMsg *>(g_00805338)->pop(
                reinterpret_cast<const char *>(g_0068a0c0), 0xffffec78, 0,
                reinterpret_cast<const char *>(g_0068a0b0));
            return;
        }
    }

    int r = morale_veh(a1, 1, 0);
    parse_say(0, g_952328_arr[r * 2], -1, -1);
    reinterpret_cast<NetMsg *>(g_00805338)->pop(
        reinterpret_cast<const char *>(g_0068a0a8), 0xffffec78, 0,
        reinterpret_cast<const char *>(g_0068a098));
}
