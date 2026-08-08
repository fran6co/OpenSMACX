// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004CB310
// name           ?action_go_to@@YAXH@Z
// size           623 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004cb310/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action_go_to@@YAXH@Z  at 0x004CB310  (623 bytes)
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
class Path;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int order_veh(int, int, int);
    void await_exec(int);
    void await_synch();
    void unlock_veh();
};
class Path { public:
    int move(int, int);
};
int __cdecl order_veh(int, int, int);
void __cdecl synch_veh(int);
void __cdecl veh_skip(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e908 = (int *)0x0093E908;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00945b00 = (int *)0x00945B00;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_00952839 = (int *)0x00952839;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009ab88c = (int *)0x009AB88C;

// Vehicle table, stride 0x34, indexed by a1 - extern array keeps the
// base-immediate + offset-register encoding instead of folding to a `lea`.
extern unsigned char g_00952828x[];
// Byte flag table, stride 0x90.
extern unsigned char g_0094a37bx[];
// Indexed by a register (short field * 0x34) - extern array, not the
// pointer-constant scaffolding form (the index is register-computed).
extern unsigned char g_009ab88cx[];

void __cdecl action_go_to(int a1) {
    int netActiveFlag = *g_0093f660;
    int hostFlag = *g_0093a938;
    if (netActiveFlag != 0 && hostFlag == 0) {
        *reinterpret_cast<const int **>(g_0093e908) = &a1;
    }

    unsigned char *rec = g_00952828x + a1 * 0x34;
    int moveResult;
    if (rec[0x11] == 0x19) {
        moveResult = *reinterpret_cast<short *>(rec + 0x14);
    } else {
        unsigned amount;
        if (rec[0x11] & 0x40) {
            amount = 0xffffffff;
        } else {
            amount = rec[0xE];
        }
        moveResult = reinterpret_cast<Path *>(g_00945b00)->move(a1, amount);
        rec = g_00952828x + a1 * 0x34;
    }

    if (moveResult < 0) {
        bool netActive = *g_0093f660 != 0;
        rec[0x11] = 0;
        if (netActive && *g_0093a938 == 0) {
            synch_veh(a1);
            reinterpret_cast<NetDaemon *>(g_0093cd90)->await_synch();
            reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
        }
    } else if (*g_0093f660 != 0 && *g_0093a938 == 0) {
        int r = reinterpret_cast<NetDaemon *>(g_0093cd90)->order_veh(a1, moveResult, 1);
        if (r != 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->await_exec(1);
            return;
        }
    } else {
        if (order_veh(a1, moveResult, 3) != 0) {
            if (rec[0x11] == 0x19) {
                if (*reinterpret_cast<short *>(rec + 0x00) == *reinterpret_cast<short *>(rec + 0x16) &&
                    *reinterpret_cast<short *>(rec + 0x02) == *reinterpret_cast<short *>(rec + 0x1e)) {
                    rec[0x11] = 0;
                    return;
                }
            } else if (rec[0x11] == 0x18 &&
                       *reinterpret_cast<short *>(rec + 0x00) == *reinterpret_cast<short *>(rec + 0x14) &&
                       *reinterpret_cast<short *>(rec + 0x02) == *reinterpret_cast<short *>(rec + 0x1c)) {
                rec[0x11] = 0;
                if ((*reinterpret_cast<int *>(rec + 0x04) & 0x2000200) == 0x2000200 &&
                    rec[0x13] < rec[0x12]) {
                    int i = 0;
                    short *wp = reinterpret_cast<short *>(rec + 0x14);
                    if (rec[0x12] != 0) {
                        do {
                            ++i;
                            wp[0] = wp[1];
                            wp[4] = wp[5];
                            ++wp;
                        } while (i < rec[0x12]);
                    }
                    rec[0x11] = 0x18;
                    char cnt = static_cast<char>(rec[0x12] - 1);
                    rec[0x12] = static_cast<unsigned char>(cnt);
                    if (cnt == 0) {
                        *reinterpret_cast<int *>(rec + 0x04) &= 0xfdfefdff;
                    }
                }
                if (!(*reinterpret_cast<unsigned char *>(g_009a64e8) & (1 << (rec[0xE] & 0x1f))) &&
                    g_0094a37bx[g_009ab88cx[*reinterpret_cast<short *>(rec + 0x0A) * 0x34] * 0x90] != 0) {
                    int idx = *reinterpret_cast<short *>(rec + 0x02) * (*g_0068faf0) +
                              (*reinterpret_cast<short *>(rec + 0x00) >> 1);
                    unsigned char *tile = *reinterpret_cast<unsigned char **>(g_0094a30c) + idx * 0x2c;
                    if ((tile[8] & 1) != 0) {
                        int v = tile[2] & 0xf;
                        if (v >= 0 && v < 8) {
                            veh_skip(a1);
                            return;
                        }
                    }
                }
            }
        }
    }
}
