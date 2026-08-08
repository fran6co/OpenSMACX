// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005C0B00
// name           ?kill@@YAXH@Z
// size           432 bytes
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005c0b00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?kill@@YAXH@Z  at 0x005C0B00  (432 bytes)
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

class BaseWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BaseWin { public:
    void check_base(int);
};
int __cdecl has_abil(int, int);
int __cdecl stack_check(int, int, int, int, int);
int __cdecl stack_veh(int, int);
int __cdecl veh_at(int, int);
void __cdecl draw_radius(int, int, int, int);
void __cdecl draw_tile(int, int, int);
void __cdecl stack_kill(int);
void __cdecl veh_kill(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_0091561c = (int *)0x0091561C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094a30c = (int *)0x0094A30C;

// Per-unit record table, stride 0x34, indexed by a1. Fields used here:
// +0x00 x-coord (short), +0x02 y-coord (short), +0x0A unit-type id (short),
// +0x0E a flag byte, +0x24 a "bump" byte, +0x2E a base id (short).
extern unsigned char g_tbl1[];
// Unit-type record table, stride 0x34, indexed by unit-type id.
extern unsigned char g_tbl2[];
// Armor/class lookup, stride 0x10, indexed by a byte from g_tbl2.
extern signed char g_tbl3[];

void __cdecl kill(int a1) {
    unsigned char *base1 = g_tbl1 + a1 * 0x34;
    int x = *reinterpret_cast<short *>(base1 + 0x00);
    int y = *reinterpret_cast<short *>(base1 + 0x02);
    int unit_type = *reinterpret_cast<short *>(base1 + 0x0A);
    int flag = base1[0x0E];
    int base_id = *reinterpret_cast<short *>(base1 + 0x2E);

    int has_ability = has_abil(unit_type, 2);

    unsigned char *base2 = g_tbl2 + unit_type * 0x34;
    int class_byte = base2[3];      // 0x9AB890 - 0x9AB88D
    int uVar8;
    if (class_byte == 0 || unit_type > 0x3f ||
        (g_tbl3[static_cast<unsigned char>(base2[0]) * 0x10] >= 0 && unit_type != 0xf)) {
        uVar8 = class_byte;
    } else {
        uVar8 = base1[0x24] + 1;
    }

    if (uVar8 == 0) {
        veh_kill(a1);
        goto after_kill;
    }

    if (y >= 0 && y < *g_00949874 && x >= 0 && x < *g_00949870) {
        int tile_index = *g_0068faf0 * y + (x >> 1);
        unsigned char tile_byte =
            (*reinterpret_cast<unsigned char **>(g_0094a30c))[tile_index * 0x2c];
        if ((tile_byte & 0xe0) >= 0x60) {
            veh_kill(a1);
            goto after_kill;
        }
    } else if (x == -2) {
        stack_kill(a1);
        return;
    }

    stack_veh(a1, 2);
    stack_kill(a1);

after_kill:
    reinterpret_cast<BaseWin *>(g_006a7628)->check_base(base_id);

    if (*g_0091561c == 0 && y >= 0 && y < *g_00949874 && x >= 0 && x < *g_00949870) {
        if (flag == *g_00939284) {
            int result = veh_at(x, y);
            int checked = stack_check(result, 1, flag, -1, -1);
            if (checked == 0 || has_ability != 0) {
                draw_radius(x, y, 2, 2);
                return;
            }
        }
        draw_tile(x, y, 2);
    }
}
