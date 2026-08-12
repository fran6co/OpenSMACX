// ORIGINAL: 0x004CD090 FILE
// name      ?action_arty@@YAXHHH@Z
// size      605 bytes
// spans     0x004CD090-0x004CD2ED
// prototype void (__cdecl ?action_arty@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   2   call targets   14
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A890 0x004CAA50 0x00506A60 0x00531020 0x005310F0 0x00531300 0x00531700 0x00592E10 0x00592EE0 0x005A65D0 0x005B8E10 0x005BFE90 0x005C1B70 0x00644F3A
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004CD090
// measured tier  MISMATCH
// divergence     42
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004cd090/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action_arty@@YAXHHH@Z  at 0x004CD090  (605 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int lock_veh(int *, int, int, int, int);
    int maybe_lock(int *, int, int, int, int, int);
    void await_exec(int);
    void unlock_veh();
};
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
extern "C" int abs(int);
int __cdecl battle_fight(int, int, int, int, int *);
int __cdecl radius_move(int, int, int, int, int);
int __cdecl stack_fix(int);
int __cdecl veh_at(int, int);
int __cdecl veh_ready(int);
void __cdecl action_destroy(int, int, int, int);
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl message_veh(int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066f8c4 = (int *)0x0066F8C4;
static int *const g_00687fe4 = (int *)0x00687FE4;
static int *const g_00687ff0 = (int *)0x00687FF0;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e904 = (int *)0x0093E904;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00949744 = (int *)0x00949744;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_00952836 = (int *)0x00952836;
static int *const g_0096c9f8 = (int *)0x0096C9F8;

// Indexed table bases: each pointer-constant spelling for these globals
// folds the scaled-index addressing under optimization because the
// ADDRESS ITSELF does the index arithmetic; declare array forms instead.
extern uint8_t g_00952836_arr[];
extern int16_t g_0095282a_arr[];
extern int16_t g_00952828_arr[];
extern uint8_t g_0096c9f8_arr[];
extern int g_0066f8c4_arr[];

// The original calls CRT `_abs` both times; without this, /O2 recognizes
// `abs` as an intrinsic and expands it inline (cdq/xor/sub) instead.
#pragma function(abs)

void __cdecl action_arty(int a1, int a2, int a3) {
    if (g_00952836_arr[a1 * 0x34] == *g_00939284 && veh_ready(a1) == 0) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(
            reinterpret_cast<const char *>(g_00687fe4), 5000, 0, 0);
        return;
    }

    int iVar3 = a2;
    int iVar2 = a3;
    int sVar1 = g_0095282a_arr[a1 * 0x1a];
    int newA2 = abs(g_00952828_arr[a1 * 0x1a] - a2);

    if ((*reinterpret_cast<uint8_t *>(g_0094988c) & 1) == 0 && newA2 > *g_0068faf0) {
        newA2 = *g_00949870 - newA2;
    }

    int iVar4 = abs(sVar1 - a3);
    if (((iVar4 + newA2) & ~1) > 4) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(
            reinterpret_cast<const char *>(g_00687ff0), 5000, 0, 0);
        return;
    }

    int veh = veh_at(iVar3, iVar2);
    int fixed = stack_fix(veh);
    if (fixed < 0) {
        if (*g_0093f660 == 0) {
            action_destroy(a1, 0, iVar3, iVar2);
            return;
        }
        int r = reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_veh(&a1, 4, iVar3, iVar2, 0);
        if (r == 0) {
            message_data(0x2413, 0, a1, iVar3, iVar2, 0);
            reinterpret_cast<NetDaemon *>(g_0093cd90)->await_exec(1);
        }
        return;
    }

    if (g_00952836_arr[fixed * 0x34] == g_00952836_arr[a1 * 0x34]) {
        return;
    }

    int idx = (static_cast<uint8_t>(g_00952836_arr[a1 * 0x34]) * 0x833 +
               static_cast<uint8_t>(g_00952836_arr[fixed * 0x34])) * 4;
    if ((g_0096c9f8_arr[idx] & 1) != 0) {
        return;
    }

    int result = radius_move(g_00952828_arr[a1 * 0x1a], g_0095282a_arr[a1 * 0x1a],
                              iVar3, iVar2, g_0066f8c4_arr[*g_00949744]);
    if (result < 0) {
        return;
    }

    if (*g_0093f660 == 0) {
        *g_0093e904 = 3;
        battle_fight(a1, result, 1, 1, 0);
        return;
    }

    int r2 = reinterpret_cast<NetDaemon *>(g_0093cd90)->maybe_lock(&a1, 0, 0, iVar3, iVar2, 4);
    if (r2 != 0) {
        return;
    }

    *g_0093e904 = 1;
    int r3 = battle_fight(a1, result, 1, 1, 0);
    if (r3 != 0) {
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
        return;
    }
    message_veh(0x2402, a1, result, 0);
    reinterpret_cast<NetDaemon *>(g_0093cd90)->await_exec(1);
}
