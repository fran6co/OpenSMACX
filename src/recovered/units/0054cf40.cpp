// ORIGINAL: 0x0054CF40 FILE
// name      ?give_a_base@@YAXHH@Z
// size      1232 bytes
// spans     0x0054CF40-0x0054D410
// prototype void (__cdecl ?give_a_base@@YAXHH@Z)(int baseID, int factionID)
// callers   5   call targets   13
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AF90 0x004E39D0 0x004E3C60 0x004EC3B0 0x00523DD0 0x00591B10 0x00591E50 0x005ADE80 0x005B57D0 0x005B8AF0 0x005B8EE0 0x005C0B00 0x00644F3A
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0054CF40
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0054cf40/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?give_a_base@@YAXHH@Z  at 0x0054CF40  (1232 bytes)
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
extern "C" int abs(int);
int __cdecl base_compute(int);
int __cdecl base_find(int, int, int);
int __cdecl stack_veh(int, int);
void __cdecl draw_tile_fixup(int, int, int, int);
void __cdecl kill(int);
void __cdecl owner_set(int, int, int);
void __cdecl replay_base(int, int, int, int);
void __cdecl reset_territory();
void __cdecl set_base(int);
void __cdecl spot_base(int, int);
void __cdecl stack_put(int, int, int);
void __cdecl synch_bit(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_0093fab0 = (int *)0x0093FAB0;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_00952832 = (int *)0x00952832;
static int *const g_00952856 = (int *)0x00952856;
static int *const g_009a64c8 = (int *)0x009A64C8;

// Base record table, stride 0x134 bytes, one indexed access per field
// each time (not a register-advanced walk) - matches 00564b90's proven
// `static char *const g_0097d040` for this exact table.
static uint8_t *const g_0097d040_base = (uint8_t *)0x0097D040;

// Unit record tables, walked via a computed base+index*0x34 - the
// indexed-table-base lever applies (extern T[], not a folded pointer).
// New names: the context's g_00952832/g_00952856 above are plain int*
// pointer constants, wrong for this register-scaled access.
extern uint8_t g_00952832_tbl[];
extern uint8_t g_00952856_tbl[];

// Neighbor-offset tables, walked by a byte-offset register directly.
extern int32_t g_0066efbc[];
extern int32_t g_0066f440[];

// Unit-type/faction lookup tables, indexed by a register-scaled offset.
extern uint8_t g_009ab892[];
extern uint8_t g_009ab88c[];
extern uint8_t g_0094a379[];

// Per-faction unit-count tables, row stride 0x20cc, indexed by unit type.
extern uint8_t g_0096d238[];
extern uint8_t g_0096d438[];

void __cdecl give_a_base(int a1, int a2) {
    int idx13 = a1 * 0x134;
    int x0 = *reinterpret_cast<int16_t *>(g_0097d040_base + idx13);
    int y0 = *reinterpret_cast<int16_t *>(g_0097d040_base + idx13 + 2);
    unsigned owner0 = *reinterpret_cast<uint8_t *>(g_0097d040_base + idx13 + 4);

    for (int off = 0; off < 0x54; off += 4) {
        int dx = *reinterpret_cast<int32_t *>(reinterpret_cast<char *>(g_0066efbc) + off) + x0;
        if ((*g_0094988c & 1) == 0) {
            if (dx < 0) {
                dx = *g_00949870 + dx;
            } else if (*g_00949870 <= dx) {
                dx = dx - *g_00949870;
            }
        }
        int dy = *reinterpret_cast<int32_t *>(reinterpret_cast<char *>(g_0066f440) + off) + y0;
        if (dy >= 0 && dy < *g_00949874 && dx >= 0 && dx < *g_00949870) {
            uint8_t *cell = *reinterpret_cast<uint8_t **>(g_0094a30c) + 4 +
                             (*g_0068faf0 * dy + (dx >> 1)) * 0x2c;
            *cell = *cell | static_cast<uint8_t>(1 << a2);
            synch_bit(dx, dy, a2);
        }
    }

    int last = *g_009a64c8 - 1;
    if (last >= 0) {
        for (int i = last; i >= 0; i--) {
            uint8_t *unit = g_00952832_tbl + i * 0x34;
            if (unit[4] != owner0) continue;

            int ux = *reinterpret_cast<int16_t *>(unit - 0xa);
            int uy = *reinterpret_cast<int16_t *>(unit - 8);

            int dx1 = abs(ux - x0);
            if ((*g_0094988c & 1) == 0 && *g_0068faf0 < dx1) {
                dx1 = *g_00949870 - dx1;
            }
            int dy1 = abs(uy - y0);
            int adx = abs(dx1);
            int ady = abs(dy1);
            int hi = (adx > ady) ? adx : ady;
            int lo = (adx < ady) ? adx : ady;
            int cheby = (((adx + ady) >> 1) - lo + 1) >> 1;
            int rem = hi - cheby;

            uint8_t *tileArray = *reinterpret_cast<uint8_t **>(g_0094a30c);
            uint8_t terrByte = tileArray[(*g_0068faf0 * uy + (ux >> 1)) * 0x2c];
            unsigned flag = ((terrByte & 0xe0) >= 0x60) ? 1u : 0u;

            if (rem <= static_cast<int>(flag)) {
                int unitType = *reinterpret_cast<int16_t *>(unit);
                bool doFind = true;
                int kind = g_009ab892[unitType * 0x34];
                if ((kind == 2 || kind == 3) && rem == 0) {
                    doFind = false;
                } else {
                    unsigned char facRaw = g_009ab88c[unitType * 0x34];
                    unsigned char factionCat = g_0094a379[facRaw * 0x90];
                    if (factionCat == 1 && (rem == 0 || (terrByte & 0xe0) >= 0x60)) {
                        doFind = false;
                    }
                }

                bool transferred = false;
                if (doFind) {
                    g_0097d040_base[idx13 + 4] = static_cast<uint8_t>(a2);
                    int foundBase = base_find(x0, y0, owner0);
                    g_0097d040_base[idx13 + 4] = static_cast<uint8_t>(owner0);
                    if (foundBase >= 0) {
                        int fbx = *reinterpret_cast<int16_t *>(g_0097d040_base + foundBase * 0x134);
                        int fby = *reinterpret_cast<int16_t *>(g_0097d040_base + foundBase * 0x134 + 2);
                        int adx2 = abs(fbx - x0);
                        if ((*g_0094988c & 1) == 0 && *g_0068faf0 < adx2) {
                            adx2 = *g_00949870 - adx2;
                        }
                        int ady2 = abs(fby - y0);
                        if ((adx2 + ady2) >= 2) {
                            stack_put(stack_veh(i, 1), fbx, fby);
                            unit[7] = 0;
                            if (*reinterpret_cast<int16_t *>(unit + 0x24) == a1) {
                                *reinterpret_cast<int16_t *>(unit + 0x24) = static_cast<int16_t>(foundBase);
                            }
                            transferred = true;
                        }
                    }
                }

                if (!transferred) {
                    int unitType2 = *reinterpret_cast<int16_t *>(unit);
                    g_0096d238[owner0 * 0x20cc + unitType2]--;
                    unit[4] = static_cast<uint8_t>(a2);
                    g_0096d238[a2 * 0x20cc + unitType2]++;
                    *reinterpret_cast<int16_t *>(unit + 0x24) = static_cast<int16_t>(a1);
                    unit[0xf] = 0;
                    if (unit[7] != 2) {
                        unit[7] = 0;
                    }
                    owner_set(ux, uy, a2);
                }
            }
        }
    }

    g_0097d040_base[idx13 + 4] = static_cast<uint8_t>(a2);
    *reinterpret_cast<int32_t *>(g_0097d040_base + idx13 + 0x9c) = 0;
    *reinterpret_cast<int32_t *>(g_0097d040_base + idx13 + 0x40) = 0;

    if (*reinterpret_cast<int32_t *>(g_0097d040_base + idx13 + 0x4c) >= 0) {
        int32_t *goals = reinterpret_cast<int32_t *>(g_0097d040_base + idx13 + 0x50);
        int n = *reinterpret_cast<int32_t *>(g_0097d040_base + idx13 + 0x4c) + 1;
        int gi = 0;
        do {
            if (goals[gi] >= 0) {
                g_0096d438[owner0 * 0x20cc + goals[gi]]--;
            }
            gi++;
            n--;
        } while (n != 0);
    }

    *reinterpret_cast<int32_t *>(g_0097d040_base + idx13 + 0x4c) = 0;
    *reinterpret_cast<int32_t *>(g_0097d040_base + idx13 + 0x44) = 0xffffffbb;
    *reinterpret_cast<int32_t *>(g_0097d040_base + idx13 + 0x50) = 0xffffffbb;

    replay_base(1, x0, y0, a2);

    int mode = *g_0093fab0;
    *reinterpret_cast<uint32_t *>(g_0097d040_base + idx13 + 0x30) &= 0xffdef7b1;
    g_0097d040_base[idx13 + 8] = 0;

    if (mode == 3) {
        if (g_0097d040_base[idx13 + 5] == static_cast<uint8_t>(a2)) {
            g_0097d040_base[idx13 + 7] = 0;
        } else {
            g_0097d040_base[idx13 + 5] = static_cast<uint8_t>(owner0);
            g_0097d040_base[idx13 + 7] = 0x32;
        }
    } else {
        if (g_0097d040_base[idx13 + 5] == static_cast<uint8_t>(a2)) {
            g_0097d040_base[idx13 + 7] = 0;
        }
        g_0097d040_base[idx13 + 5] = static_cast<uint8_t>(a2);
    }

    spot_base(a1, owner0);
    reset_territory();

    for (int j = *g_009a64c8 - 1; j >= 0; j--) {
        uint8_t *unit2 = g_00952856_tbl + j * 0x34;
        if (j < *g_009a64c8 && unit2[-0x20] == owner0 &&
            *reinterpret_cast<int16_t *>(unit2) == a1) {
            kill(j);
        }
    }

    owner_set(x0, y0, a2);
    draw_tile_fixup(x0, y0, 1, 2);
    set_base(a1);
    base_compute(1);
}
