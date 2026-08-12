// ORIGINAL: 0x004D7BE0 FILE
// name      ?road_to@Console@@QAEXHH@Z
// size      269 bytes
// spans     0x004D7BE0-0x004D7CED
// prototype void (__thiscall ?road_to@Console@@QAEXHH@Z)(Console* this, int, int)
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00510490 0x005B9F20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004D7BE0
// measured tier  MISMATCH
// divergence     43
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004d7be0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?road_to@Console@@QAEXHH@Z  at 0x004D7BE0  (269 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under BOTH cl 12.00.8168 and
// i686-w64-mingw32-g++ -std=c++11. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

struct int8;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetMsg { public:
    void pop(int8*, int, int, int8*);
};
int __cdecl has_tech(int techID, int factionID);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00688e18 = (int *)0x00688E18;
static int *const g_00688e2c = (int *)0x00688E2C;
static int *const g_00688e38 = (int *)0x00688E38;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00691940 = (int *)0x00691940;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_0094a30c = (int *)0x0094A30C;

// Vehs[veh_id] - matches src/veh.h's real `Veh` layout (x@0, y@2, proto_id@10,
// faction_id@14, sizeof 52) closely enough for the fields this body touches.
struct VehLite {
    short x;
    short y;
    char pad1[6];
    short proto_id;
    char pad2[2];
    unsigned char faction_id;
    char pad3[37];
};

// VehPrototypes[proto_id] - chassis_id@36, plan@42, sizeof 52.
struct VehPrototypeLite {
    char pad1[36];
    unsigned char chassis_id;
    char pad2[5];
    unsigned char plan;
    char pad3[9];
};

// Chassis[chassis_id] - triad@73, sizeof 144.
struct RulesChassisLite {
    char pad1[73];
    unsigned char triad;
    char pad2[70];
};

// (*MapTiles)[tile_index] - climate@0, sizeof 44.
struct MapLite {
    unsigned char climate;
    char pad[43];
};

static VehLite *const g_00952828 = (VehLite *)0x00952828;
static VehPrototypeLite *const g_009ab868 = (VehPrototypeLite *)0x009AB868;
static RulesChassisLite *const g_0094a330 = (RulesChassisLite *)0x0094A330;

class Console { public:
    void cursor_on(int, int);
    void road_to(int, int);
};

void Console::road_to(int a1, int a2) {
    // Plain-int indices, indexed fresh at each use: `array[i].field` keeps the
    // compiler's shared value as JUST the scaled index (base + field offset
    // both fold into the instruction's own displacement). Storing `&array[i]`
    // in a typed pointer instead forces the base into the register up front,
    // which is a different (and, measured, non-matching) instruction shape.
    int protoId = g_00952828[a1].proto_id;
    if (g_009ab868[protoId].plan != 9) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(
            reinterpret_cast<int8 *>(g_00688e18), 0x1388, 0, 0);
        return;
    }
    int chassisId = g_009ab868[protoId].chassis_id;
    if (g_0094a330[chassisId].triad != 0) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(
            reinterpret_cast<int8 *>(g_00688e2c), 0x1388, 0, 0);
        return;
    }
    int climate = (*reinterpret_cast<MapLite **>(g_0094a30c))[
        (g_00952828[a1].x >> 1) + g_00952828[a1].y * (*g_0068faf0)].climate;
    if ((climate & ~0x1F) < 0x60) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(
            reinterpret_cast<int8 *>(g_00688e38), 0x1388, 0, 0);
        return;
    }
    if (a2 != 0x1c || has_tech(*g_00691940, g_00952828[a1].faction_id)) {
        reinterpret_cast<Console *>(g_009156b0)->cursor_on((a2 == 0x1c) + 2, -1);
    }
}
