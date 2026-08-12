// ORIGINAL: 0x004E68B0 FILE
// name      ?farm_compute@@YAXXZ
// size      1438 bytes
// spans     0x004E68B0-0x004E6E4E
// prototype 
// callers   1   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0 0x00579A30 0x00592140 0x005B8E10 0x005B9580 0x005BFE90 0x00644F3A
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004E68B0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e68b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?farm_compute@@YAXXZ  at 0x004E68B0  (1438 bytes)
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
int __cdecl goody_at(int, int);
int __cdecl stack_check(int, int, int, int, int);
int __cdecl stack_fix(int);
int __cdecl veh_at(int, int);
int __cdecl whose_territory(int, int, int, int *, int);
void __cdecl add_goal(int, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066efbc = (int *)0x0066EFBC;
static int *const g_0066f440 = (int *)0x0066F440;
static int *const g_00689370 = (int *)0x00689370;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_0090e99c = (int *)0x0090E99C;
static int *const g_0090e9f0 = (int *)0x0090E9F0;
static int *const g_0090ea00 = (int *)0x0090EA00;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_0090ea34 = (int *)0x0090EA34;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0097d040 = (int *)0x0097D040;
static int *const g_009a64cc = (int *)0x009A64CC;

// Indexed table bases: each address does arithmetic work (index scaled
// by a computed register), so fresh externs are used instead of the
// scaffold's const-pointer spellings for these.
extern int g_x66efbc[];
extern int g_x66f440[];
extern int g_x90e99c[];
extern unsigned int g_x96c9e0[];
extern unsigned int g_x96c9f8[];
extern short g_x95285a[];
extern short g_x952858[];
extern unsigned char g_x952836[];
extern unsigned char g_x95284f[];
extern short g_x97d040[];

void __cdecl farm_compute() {
    short *cityPos = reinterpret_cast<short *>(g_0090ea30);
    int cx = cityPos[0];
    int cy = cityPos[1];
    unsigned char faction = *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(g_0090ea30) + 4);
    unsigned int uVar12 = faction;
    unsigned char *terrain = *reinterpret_cast<unsigned char **>(g_0094a30c);

    for (int local_c = 0; local_c < 100; local_c += 4) {
        int idx = local_c / 4;
        if (local_c < 0x54) {
            int x = g_x66efbc[idx] + cx;
            if ((*g_0094988c & 1) == 0) {
                if (x < 0) {
                    x = x + *g_00949870;
                } else if (*g_00949870 <= x) {
                    x = x - *g_00949870;
                }
            }
            int y = g_x66f440[idx] + cy;
            if (y < 0 || *g_00949874 <= y || x < 0 || *g_00949870 <= x) {
                g_x90e99c[idx] = 2;
            } else {
                int half = x >> 1;
                if (local_c < 0x54 && *g_0090ea34 == 1) {
                    int gv = goody_at(x, y);
                    if (gv != 0 && (terrain[(*g_0068faf0 * y + half) * 0x2c] & 0xe0) > 0x5f) {
                        add_goal(uVar12, 3, 1, x, y, -1);
                        add_goal(uVar12, 2, 1, x, y, -1);
                        add_goal(uVar12, 0, 1, x, y, -1);
                    }
                }
                if ((g_x96c9e0[uVar12 * 0x833] & 0x200) == 0 &&
                    (terrain[4 + (*g_0068faf0 * y + half) * 0x2c] & (1 << (faction & 0x1f))) == 0) {
                    g_x90e99c[idx] = 2;
                } else {
                    g_x90e99c[idx] = 0;
                    unsigned char *rec = terrain + (*g_0068faf0 * y + half) * 0x2c;
                    if ((rec[8] & 1) != 0 && (rec[2] & 0xf) < 8) {
                        g_x90e99c[idx] = 4;
                    }
                    int vehId = veh_at(x, y);
                    int stackId = stack_fix(vehId);
                    if (stackId >= 0) {
                        int sc = stack_check(stackId, 9, 3, -1, -1);
                        if (sc != 0) {
                            g_x90e99c[idx] = g_x90e99c[idx] | 8;
                        }
                        short chain = g_x95285a[stackId * 0x1a];
                        while (chain >= 0) {
                            stackId = chain;
                            chain = g_x95285a[stackId * 0x1a];
                        }
                        for (; stackId >= 0; stackId = static_cast<short>(g_x952858[stackId * 0x1a])) {
                            if (g_x952836[stackId * 0x34] != uVar12 &&
                                (g_x95284f[stackId * 0x34] & (1 << (faction & 0x1f))) != 0 &&
                                (g_x96c9f8[g_x952836[stackId * 0x34] * 4 + uVar12 * 0x833] & 3) == 0) {
                                g_x90e99c[idx] = g_x90e99c[idx] | 8;
                            }
                        }
                    }
                    int who = 0;
                    unsigned int terr = whose_territory(uVar12, x, y, 0, 0);
                    if (static_cast<int>(terr) >= 0 && terr != uVar12 &&
                        ((g_x96c9f8[terr * 4 + uVar12 * 0x833] & 3) != 0 ||
                         (stackId < 0 || g_x952836[stackId * 0x34] != uVar12))) {
                        g_x90e99c[idx] = g_x90e99c[idx] | 0x20;
                    }
                    (void)who;
                }
            }
        } else {
            g_x90e99c[idx] = 1;
        }
    }

    if (*g_009a64cc > 0) {
        short *other = g_x97d040;
        for (int cityIdx = 0; cityIdx < *g_009a64cc; cityIdx++, other += 0x9a) {
            if (cityIdx != *g_00689370) {
                int ox = other[0];
                int oy = other[1];
                int dx = abs(cx - ox);
                if ((*g_0094988c & 1) == 0 && *g_0068faf0 < dx) {
                    dx = *g_00949870 - dx;
                }
                int dy = abs(oy - cy);
                int adx = abs(dx);
                int ady = abs(dy);
                int mx = (adx > ady) ? adx : ady;
                int mn = (ady > adx) ? adx : ady;
                if (mx - (((adx + ady) >> 1) - mn + 1 >> 1) < 6) {
                    int mapH = *g_00949870;
                    for (int local_14 = 0; local_14 < 0x15; local_14++) {
                        unsigned int rflags = *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(other) + 0x3c);
                        if (local_14 == 0 || (rflags & (1u << (local_14 & 0x1f))) != 0) {
                            int x2 = g_x66efbc[local_14] + ox;
                            if ((*g_0094988c & 1) == 0) {
                                if (x2 < 0) {
                                    x2 = x2 + mapH;
                                } else if (mapH <= x2) {
                                    x2 = x2 - mapH;
                                }
                            }
                            int y2 = g_x66f440[local_14] + oy;
                            if (y2 >= 0 && y2 < *g_00949874 && x2 >= 0 && x2 < mapH &&
                                (*reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(other) + 4) == uVar12 ||
                                 (terrain[(*g_0068faf0 * y2 + (x2 >> 1)) * 0x2c] & 0xe0) < 0x60)) {
                                int ddx = abs(cx - x2);
                                if ((*g_0094988c & 1) == 0 && *g_0068faf0 < ddx) {
                                    ddx = mapH - ddx;
                                }
                                int ddy = abs(cy - y2);
                                int a7 = abs(ddx);
                                int a8 = abs(ddy);
                                int mx2 = (a7 > a8) ? a7 : a8;
                                int mn2 = (a8 > a7) ? a7 : a8;
                                if (mx2 - (((a7 + a8) >> 1) - mn2 + 1 >> 1) < 3) {
                                    for (int local_c2 = 0; local_c2 < 0x54; local_c2 += 4) {
                                        int idx2 = local_c2 / 4;
                                        int x3 = g_x66efbc[idx2] + cx;
                                        if ((*g_0094988c & 1) == 0) {
                                            if (x3 < 0) {
                                                x3 = x3 + mapH;
                                            } else if (mapH <= x3) {
                                                x3 = x3 - mapH;
                                            }
                                        }
                                        if (x3 == x2 && g_x66f440[idx2] + cy == y2) {
                                            g_x90e99c[idx2] = g_x90e99c[idx2] | 0x10;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    *g_0090ea00 = 0;
    unsigned char bit = 0;
    int *out = g_x90e99c;
    while (reinterpret_cast<char *>(out) < reinterpret_cast<char *>(g_0090e9f0)) {
        if (*out != 0) {
            *g_0090ea00 = *g_0090ea00 | (1 << (bit & 0x1f));
        }
        out++;
        bit++;
    }
}
