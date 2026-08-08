// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004E5FE0
// name           ?base_reset@@YAXHH@Z
// size           795 bytes
// measured tier  MISMATCH
// divergence     8
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e5fe0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_reset@@YAXHH@Z  at 0x004E5FE0  (795 bytes)
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
int __cdecl base_build(int, int *, int *, int *);
int __cdecl parse_say(int, int, int, int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
void __cdecl base_change(int, int);
void __cdecl help_topic(unsigned int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00689484 = (int *)0x00689484;
static int *const g_00689494 = (int *)0x00689494;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_0090ea44 = (int *)0x0090EA44;
static int *const g_0090ea48 = (int *)0x0090EA48;
static int *const g_0090ea4c = (int *)0x0090EA4C;
static int *const g_0090ea50 = (int *)0x0090EA50;
static int *const g_0090ea54 = (int *)0x0090EA54;
static int *const g_0090ea58 = (int *)0x0090EA58;
static int *const g_0090ea5c = (int *)0x0090EA5C;
static int *const g_0090ea60 = (int *)0x0090EA60;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0096cbb0 = (int *)0x0096CBB0;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64e8 = (int *)0x009A64E8;

// Per-base struct array, stride 0x134 bytes / 0x4D ints, indexed by base id.
extern unsigned char g_0097d044_tbl[];   // struct+0x00: owning faction id (byte)
extern unsigned int  g_0097d078_tbl[];   // struct+0x34: state flag bits (int)
extern int           g_0097d090_tbl[];   // struct+0x4C: location value (int)
extern char          g_0097d053_tbl[];   // struct+0x0F: name buffer (char[])
// Lookup table, stride 0xC ints.
extern int g_009a4b68_tbl[];
// Per-faction struct array, stride 0x20CC bytes / 0x833 ints, indexed by faction id.
extern unsigned int g_0096cbb0_tbl[];

void __cdecl base_reset(int a1, int a2) {
    int base_idx = a1;
    int loop_count = 0;

    unsigned char faction = g_0097d044_tbl[base_idx * 0x134];
    unsigned char faction_bit = static_cast<unsigned char>(1 << faction);
    unsigned char active_mask = *reinterpret_cast<unsigned char *>(g_009a64e8);

    unsigned int state = g_0097d078_tbl[base_idx * 0x4d];

    if ((faction_bit & active_mask) != 0 && (state & 0x80000000u) != 0 && a2 != 0) {
        *g_0090ea44 = 0;
        *g_0090ea54 = 999999;
        *g_0090ea48 = 0;
        *g_0090ea58 = 999999;
        *g_0090ea4c = 0;
        *g_0090ea5c = 999999;
        *g_0090ea50 = 0;
        *g_0090ea60 = 999999;

        if ((state & 0xf000000u) != 0) {
            for (;;) {
                g_0097d078_tbl[base_idx * 0x4d] =
                    (g_0097d078_tbl[base_idx * 0x4d] & 0xf1ffffffu) | 0x1000000u;
                *g_0090ea44 = base_build(base_idx, 0, 0, g_0090ea54);
                g_0097d078_tbl[base_idx * 0x4d] =
                    (g_0097d078_tbl[base_idx * 0x4d] & 0xf2ffffffu) | 0x2000000u;
                *g_0090ea48 = base_build(base_idx, 0, 0, g_0090ea58);
                if (*g_0090ea44 == *g_0090ea48) {
                    loop_count++;
                    if (loop_count < 8)
                        continue;
                }
                g_0097d078_tbl[base_idx * 0x4d] =
                    (g_0097d078_tbl[base_idx * 0x4d] & 0xf4ffffffu) | 0x4000000u;
                *g_0090ea4c = base_build(base_idx, 0, 0, g_0090ea5c);
                if ((*g_0090ea44 != *g_0090ea4c && *g_0090ea48 != *g_0090ea4c) ||
                    (++loop_count, loop_count > 7)) {
                    g_0097d078_tbl[base_idx * 0x4d] =
                        (g_0097d078_tbl[base_idx * 0x4d] & 0xf8ffffffu) | 0x8000000u;
                    *g_0090ea50 = base_build(base_idx, 0, 0, g_0090ea60);
                    if ((*g_0090ea44 != *g_0090ea50 && *g_0090ea48 != *g_0090ea50 &&
                         *g_0090ea4c != *g_0090ea50) ||
                        (++loop_count, loop_count > 7)) {
                        g_0097d078_tbl[base_idx * 0x4d] = state;
                        break;
                    }
                }
            }
        }

        int result = base_build(base_idx, 0, 0, &loop_count);
        base_change(base_idx, result);
        return;
    }

    int loc = g_0097d090_tbl[base_idx * 0x4d];
    int result = base_build(base_idx, 0, 0, 0);
    base_change(base_idx, result);

    active_mask = *reinterpret_cast<unsigned char *>(g_009a64e8);
    if ((faction_bit & active_mask) == 0)
        return;
    if ((g_0097d078_tbl[base_idx * 0x4d] & 0x80000000u) == 0)
        return;
    if (loc <= -0x46)
        return;
    loc = g_0097d090_tbl[base_idx * 0x4d];
    if (loc > -0x46)
        return;
    if (*g_0093f660 != 0)
        return;
    if ((*reinterpret_cast<unsigned char *>(g_009a64c0) & 1) != 0)
        return;

    int abs_loc = loc < 0 ? -loc : loc;
    char *name = &g_0097d053_tbl[base_idx * 0x134];
    int say_result;
    for (;;) {
        parse_says(0, name, -1, -1);
        say_result = parse_say(1, g_009a4b68_tbl[abs_loc * 0xc], -1, -1);
        int popp_result = popp(reinterpret_cast<char *>(*reinterpret_cast<int *>(g_00691b0c)),
                                reinterpret_cast<char *>(g_00689494), 0,
                                reinterpret_cast<char *>(g_00689484), 0);
        if (popp_result != 3)
            break;
        help_topic(0xb, abs_loc);
    }

    if (say_result == 0)
        return;

    result = base_build(-1 - base_idx, 0, 0, 0);
    base_change(base_idx, result);

    if (say_result <= 1)
        return;

    int *player = *reinterpret_cast<int **>(g_0090ea30);
    player[0x38 / 4] &= 0xfffbffffu;
    g_0096cbb0_tbl[faction * 2099] &= 0xfffbffffu;
}
