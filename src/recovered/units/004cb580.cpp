// ORIGINAL: 0x004CB580 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004CB580
// name           ?action_road_to@@YAXH@Z
// size           1304 bytes
// measured tier  MISMATCH
// divergence     16
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004cb580/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action_road_to@@YAXH@Z  at 0x004CB580  (1304 bytes)
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
class Path;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int order_veh(int, int, int);
    void await_exec(int);
    void await_synch();
    void unlock_veh();
};
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
class Path { public:
    int find(int, int, int, int, int, int, int, int);
};
bool __cdecl has_tech(int, int);
int __cdecl action_terraform(int, int, int);
int __cdecl order_veh(int, int, int);
int __cdecl parse_says(int, char *, int, int);
void __cdecl message_data(int, int, int, int, int, int);
void __cdecl say_loc(char *, int, int, int, int, int);
void __cdecl synch_veh(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00687e20 = (int *)0x00687E20;
static int *const g_00687e2c = (int *)0x00687E2C;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00691940 = (int *)0x00691940;
static int *const g_006919c0 = (int *)0x006919C0;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e908 = (int *)0x0093E908;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00945b00 = (int *)0x00945B00;
static int *const g_0094977c = (int *)0x0094977C;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0095282c = (int *)0x0095282C;
static int *const g_00952839 = (int *)0x00952839;
static int *const g_009b86a0 = (int *)0x009B86A0;

// Unit record tables (same family as 0x00952832 elsewhere): x/y/goalX/goalY
// are short fields walked with a register-scaled base+index, so the
// indexed-table-base lever applies (extern T[], not a folded pointer).
extern int16_t g_00952828[];  // x,        stride 0x1a shorts (0x34 bytes)
extern int16_t g_0095282a[];  // y,        stride 0x1a shorts
extern int16_t g_0095283c[];  // goal x,   stride 0x1a shorts
extern int16_t g_00952844[];  // goal y,   stride 0x1a shorts
extern uint8_t g_00952836_b[];  // owner byte,       stride 0x34 bytes
extern uint8_t g_00952839_b[];  // order-state byte, stride 0x34 bytes
extern uint32_t g_0095282c_f[]; // flags dword,      stride 0xd dwords (0x34 bytes)

void __cdecl action_road_to(int a1) {
    if (*g_0093f660 != 0 && *g_0093a938 == 0) {
        *g_0093e908 = reinterpret_cast<int>(&a1);
    }

    unsigned owner = g_00952836_b[a1 * 0x34];
    int goalX = g_0095283c[a1 * 0x1a];
    int y = g_0095282a[a1 * 0x1a];
    int goalY = g_00952844[a1 * 0x1a];
    int x = g_00952828[a1 * 0x1a];
    int halfX = x >> 1;

    uint8_t *tile = *reinterpret_cast<uint8_t **>(g_0094a30c) + (*g_0068faf0 * y + halfX) * 0x2c;
    uint8_t elevation = tile[0] & 0xe0;

    int netFlag;

    if (elevation < 0x60 || goalY < 0 || goalY >= *g_00949874 || goalX < 0 || goalX >= *g_00949870) {
        g_00952839_b[a1 * 0x34] = 0;
        netFlag = *g_0093f660;
    } else {
        bool didTube = false;

        if ((tile[8] & 0x20) != 0 && elevation > 0x3f &&
            !has_tech(*g_0094977c, owner) && has_tech(*g_006919c0, owner)) {
            uint8_t *tile2 = *reinterpret_cast<uint8_t **>(g_0094a30c) + (*g_0068faf0 * y + halfX) * 0x2c;
            if ((tile2[8] & 1) == 0 || (tile2[2] & 0xf) > 7) {
                if (*g_0093f660 == 0 || *g_0093a938 != 0) {
                    uint8_t savedState = g_00952839_b[a1 * 0x34];
                    action_terraform(a1, 10, 1);
                    g_00952839_b[a1 * 0x34] = savedState;
                    return;
                }
                int savedState = static_cast<int8_t>(g_00952839_b[a1 * 0x34]);
                message_data(0x2405, 0, a1, 0xe, savedState, 0);
                reinterpret_cast<NetDaemon *>(g_0093cd90)->await_exec(1);
                return;
            }
            didTube = true;
        }

        if (!didTube) {
            uint8_t *tile3 = *reinterpret_cast<uint8_t **>(g_0094a30c) + (*g_0068faf0 * y + halfX) * 0x2c;
            if (((tile3[8] & 0x20) == 0 || (tile3[0] & 0xe0) < 0x40 || has_tech(*g_0094977c, owner)) &&
                ((tile3[8] & 1) == 0 || (tile3[2] & 0xf) > 7)) {
                if ((tile3[8] & 4) == 0) {
                    if (*g_0093f660 == 0 || *g_0093a938 != 0) {
                        uint8_t savedState = g_00952839_b[a1 * 0x34];
                        action_terraform(a1, 5, 1);
                        g_00952839_b[a1 * 0x34] = savedState;
                        return;
                    }
                    int savedState = static_cast<int8_t>(g_00952839_b[a1 * 0x34]);
                    message_data(0x2405, 0, a1, 9, savedState, 0);
                    reinterpret_cast<NetDaemon *>(g_0093cd90)->await_exec(1);
                    return;
                }
                if (g_00952839_b[a1 * 0x34] == 0x1c && (tile3[8] & 8) == 0 &&
                    has_tech(*g_00691940, owner)) {
                    if (*g_0093f660 == 0 || *g_0093a938 != 0) {
                        action_terraform(a1, 5, 1);
                        g_00952839_b[a1 * 0x34] = 0x1c;
                        return;
                    }
                    message_data(0x2405, 0, a1, 9, 0x1c, 0);
                    reinterpret_cast<NetDaemon *>(g_0093cd90)->await_exec(1);
                    return;
                }
            }
        }

        if (x == goalX && y == goalY) {
            uint8_t orderState = g_00952839_b[a1 * 0x34];
            g_00952839_b[a1 * 0x34] = 0;
            if (*g_0093f660 != 0 && *g_0093a938 == 0) {
                synch_veh(a1);
                reinterpret_cast<NetDaemon *>(g_0093cd90)->await_synch();
                reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
            }
            if ((g_0095282c_f[a1 * 0xd] & 0x200) != 0) {
                return;
            }
            if (owner != static_cast<unsigned>(*g_00939284)) {
                return;
            }
            *reinterpret_cast<uint8_t *>(g_009b86a0) = 0;
            say_loc(reinterpret_cast<char *>(g_009b86a0), goalX, goalY, 0, 2, 0);
            parse_says(0, reinterpret_cast<char *>(g_009b86a0), -1, -1);
            if (orderState == 0x1b) {
                reinterpret_cast<NetMsg *>(g_00805338)->pop(
                    reinterpret_cast<const char *>(g_00687e20), 5000, 0, 0);
                return;
            }
            reinterpret_cast<NetMsg *>(g_00805338)->pop(
                reinterpret_cast<const char *>(g_00687e2c), 5000, 0, 0);
            return;
        }

        int minSpeed = has_tech(*g_0094977c, owner) ? 0 : 0x10;
        int found = reinterpret_cast<Path *>(g_00945b00)->find(x, y, goalX, goalY, 0, -1, minSpeed, -1);
        netFlag = *g_0093f660;
        if (found >= 0 && found < 8) {
            if (*g_0093f660 != 0 && *g_0093a938 == 0) {
                int r = reinterpret_cast<NetDaemon *>(g_0093cd90)->order_veh(a1, found, 1);
                if (r == 0) {
                    return;
                }
                reinterpret_cast<NetDaemon *>(g_0093cd90)->await_exec(1);
                return;
            }
            order_veh(a1, found, 3);
            return;
        }
        g_00952839_b[a1 * 0x34] = 0;
    }

    if (netFlag != 0 && *g_0093a938 == 0) {
        synch_veh(a1);
        reinterpret_cast<NetDaemon *>(g_0093cd90)->await_synch();
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
    }
}
