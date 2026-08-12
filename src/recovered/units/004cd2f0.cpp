// ORIGINAL: 0x004CD2F0 FILE
// name      ?action_destruct@@YAXH@Z
// size      481 bytes
// spans     0x004CD2F0-0x004CD4D1
// prototype void (__cdecl ?action_destruct@@YAXH@Z)(int vehID)
// callers   2   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x00504AA0 0x0057D360 0x005B8E10 0x005B8EE0 0x005B9510 0x005BFE90 0x005C0B00
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004CD2F0
// measured tier  MISMATCH
// divergence     60
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004cd2f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action_destruct@@YAXH@Z  at 0x004CD2F0  (481 bytes)
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
int __cdecl base_at(int, int);
int __cdecl boom(int, int, int);
int __cdecl stack_fix(int);
int __cdecl stack_veh(int, int);
int __cdecl veh_at(int, int);
int __cdecl weap_val(int, int);
void __cdecl kill(int);
void __cdecl stack_kill(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_009ab88f = (int *)0x009AB88F;
// ---- indexed table bases: the ADDRESS itself does work (SIB-scaled), so
// these are `extern T name[]`, not the const-pointer spelling above, which
// /O2 would fold into the addressing and change the instruction shape.
// Distinct names from the scaffold's own g_009ab88f: that declaration is a
// plain scalar pointer and is wrong for this scale-4 indexed use, but it
// cannot be edited out from under a --body candidate, so it is left unused
// and a correctly-typed alias is declared here instead.
extern int g_0066efbc[];
extern int g_0066f440[];
extern int16_t g_veh_x[];
extern int16_t g_veh_y[];
extern int16_t g_veh_proto[];
extern uint8_t g_veh_faction[];
extern uint8_t g_veh_damage[];
extern int16_t g_veh_next[];
extern uint8_t g_proto_cost[];
extern uint8_t g_proto_class[];

void __cdecl action_destruct(int a1) {
    int x = g_veh_x[a1 * 0x1a];
    int y = g_veh_y[a1 * 0x1a];
    boom(x, y, 0);

    int protoId = g_veh_proto[a1 * 0x1a];
    int factionId = g_veh_faction[a1 * 0x34];
    int weapVal = weap_val(protoId, factionId);
    int wclamp;
    if (weapVal >= 1) {
        if (weapVal > 0x14) {
            wclamp = 0x14;
        } else {
            wclamp = weapVal;
        }
    } else {
        wclamp = 1;
    }

    int cost = g_proto_cost[g_veh_proto[a1 * 0x1a] * 0x34];
    char damage = (char)((cost * wclamp) / 2);

    kill(a1);

    a1 = 0;
    int mapW = *g_00949870;
    while (a1 < 0x24) {
        int dx = *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0066efbc) + a1);
        int cx = dx + x;
        if ((*g_0094988c & 1) == 0) {
            if (cx < 0) {
                cx += mapW;
            } else if (mapW <= cx) {
                cx -= mapW;
            }
        }
        int dy = *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0066f440) + a1);
        int cy = dy + y;

        if (cy < 0) goto next_tile;
        if (*g_00949874 <= cy) goto next_tile;
        if (cx < 0) goto next_tile;
        if (mapW <= cx) goto next_tile;

        if (base_at(cx, cy) >= 0) goto next_tile;
        mapW = *g_00949870;

        {
            int v = veh_at(cx, cy);
            if (v >= 0) {
                do {
                    g_veh_damage[v * 0x34] = g_veh_damage[v * 0x34] + damage;
                    v = g_veh_next[v * 0x1a];
                } while (v >= 0);
            }
        }

    search_stack:
        {
            int v = veh_at(cx, cy);
            v = stack_fix(v);
            if (v < 0) goto next_tile;

        walk_stack:
            {
                int pid = g_veh_proto[v * 0x1a];
                int threshold;
                if (g_proto_class[pid * 0x34] == 0x0c) {
                    threshold = 1;
                } else {
                    int c = g_proto_cost[pid * 0x34];
                    if (c < 1) {
                        c = 1;
                    } else if (c > 0x64) {
                        c = 0x64;
                    }
                    threshold = c * 10;
                }
                if ((unsigned char)g_veh_damage[v * 0x34] >= threshold) {
                    stack_veh(v, 1);
                    stack_kill(v);
                    goto search_stack;
                }
                int next = g_veh_next[v * 0x1a];
                if (next < 0) {
                    goto next_tile;
                }
                v = next;
                goto walk_stack;
            }
        }

    next_tile:
        a1 += 4;
    }
}
