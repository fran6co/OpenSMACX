// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0047B760
// name           ?on_left_click@NetWin@@QAEXHH@Z
// size           918 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0047b760/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_click@NetWin@@QAEXHH@Z  at 0x0047B760  (918 bytes)
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

class AlphaNet;
class Net;
class NetWin;
class Spot;

// ---- callees, declared and never defined (a definition would be inlined) ----
class AlphaNet { public:
    uint32_t alignment_;
    uint8_t data_[0x149C];
    int pid_2_idx(unsigned long);
};
class Net { public:
    int send(void *, int, unsigned long, int);
};
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    int check(int, int, int *, int *);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0047baf8 = (int *)0x0047BAF8;
static int *const g_0047bb10 = (int *)0x0047BB10;
static int *const g_0047bb34 = (int *)0x0047BB34;
static int *const g_00800000 = (int *)0x00800000;
static int *const g_0090dd14 = (int *)0x0090DD14;
static int *const g_0090e778 = (int *)0x0090E778;
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_0090e8ec = (int *)0x0090E8EC;
static int *const g_0090e8f0 = (int *)0x0090E8F0;
static int *const g_0090e8f4 = (int *)0x0090E8F4;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_0093d6b8 = (int *)0x0093D6B8;
static int *const g_0093e8c0 = (int *)0x0093E8C0;

class NetWin { public:
    void pick_player();
    void pick_faction(int);
    int pick_pass(int);
    void pick_diff(int);
    void pick_map();
    void pick_num_players();
    void pick_global_diff();
    void pick_time();
    void pick_planet_size();
    void pick_ocean();
    void pick_erosion();
    void pick_native();
    void pick_clouds();
    void on_left_click(int, int);
};

// The address itself does work here (read-modify-write, or walked as a table
// base), so these need `extern T name;` / `extern T name[]` rather than the
// scaffold's folded pointer constant. Distinct names from the scaffold's
// `g_ADDR` ones to avoid redeclaring them with a different type.
extern int raw_0090e8ec;
extern int raw_0090e8f0;
extern int raw_0090e8f4;
extern int raw_0093d6b8[];
extern unsigned char raw_0090dd14[];

extern "C" void *memcpy(void *, const void *, unsigned int);

void NetWin::on_left_click(int a1, int a2) {
    Spot *spot = reinterpret_cast<Spot *>(reinterpret_cast<char *>(this) + 0xd34);
    spot->check(a1, a2, &a2, &a1);

    switch (a1) {
    case 0:
        switch (a2) {
        case 0:
            if (*g_0093e8c0) { pick_map(); return; }
            break;
        case 1:
            if (*g_0093e8c0) { pick_num_players(); return; }
            break;
        case 2:
            if (*g_0093e8c0) { pick_global_diff(); return; }
            break;
        case 3:
            if (*g_0093e8c0) { pick_time(); return; }
            break;
        case 4:
            if (*g_0093e8c0) { pick_planet_size(); return; }
            break;
        case 5:
            if (*g_0093e8c0) { pick_ocean(); return; }
            break;
        case 6:
            if (*g_0093e8c0) { pick_erosion(); return; }
            break;
        case 7:
            if (*g_0093e8c0) { pick_native(); return; }
            break;
        case 8:
            if (*g_0093e8c0) { pick_clouds(); return; }
            break;
        }
        break;
    case 1:
        if (*g_0093e8c0) {
            switch (a2) {
            case 0: raw_0090e8ec ^= 0x800; break;
            case 1: raw_0090e8ec ^= 2; break;
            case 2: raw_0090e8ec ^= 8; break;
            case 3: raw_0090e8ec ^= 4; break;
            case 4: raw_0090e8ec ^= 0x1000; break;
            case 5: raw_0090e8ec ^= 1; break;
            case 6: raw_0090e8ec ^= 0x10; break;
            case 7: raw_0090e8ec ^= 0x20; break;
            case 8: raw_0090e8ec ^= 0x4000; break;
            case 9: raw_0090e8ec ^= 0x200; break;
            case 10: raw_0090e8ec ^= 0x40; break;
            case 11: raw_0090e8ec ^= 0x100; break;
            case 12: raw_0090e8ec ^= 0x2000; break;
            case 13: raw_0090e8ec ^= 0x8000; break;
            case 14: raw_0090e8ec ^= 0x80; break;
            case 16: raw_0090e8f0 ^= 0x800000; break;
            case 17: raw_0090e8f0 ^= 0x1000000; break;
            case 100: raw_0090e8f4 ^= 0x10; break;
            }

            reinterpret_cast<AlphaNet *>(0x93cd90)->pid_2_idx(*g_0093d4f0);

            short local_2c[8];
            int local_1c[6];
            memcpy(local_1c, reinterpret_cast<void *>(0x90e8e0), sizeof(local_1c));
            local_2c[0] = 0x2f02;
            reinterpret_cast<Net *>(0x93cd90)->send(local_2c, 0x28, *g_0093d4f4, 1);
            return;
        }
        break;
    case 2:
        if (raw_0093d6b8[a2 * 0x67] == *g_0093d4f0) {
            pick_player();
            return;
        }
        break;
    case 3:
        if (raw_0093d6b8[a2 * 0x67] == *g_0093d4f0 ||
            (raw_0090dd14[a2 * 0x17c] == 0xff && *g_0093d4f0 == *g_0093d4f4)) {
            pick_faction(a2 + 1);
            return;
        }
        break;
    case 4:
        if (raw_0093d6b8[a2 * 0x67] == *g_0093d4f0 ||
            (raw_0090dd14[a2 * 0x17c] == 0xff && *g_0093d4f0 == *g_0093d4f4)) {
            pick_diff(a2 + 1);
        }
        break;
    case 5:
        if ((*g_0090e778 == 0 || *g_0090e778 == 1) &&
            raw_0093d6b8[a2 * 0x67] == *g_0093d4f0) {
            pick_pass(0);
            return;
        }
        break;
    }
}
