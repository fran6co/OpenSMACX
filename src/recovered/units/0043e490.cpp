// ORIGINAL: 0x0043E490 ?setup_veh@DesignWin@@QAEXHH@Z 0x0043E490-0x0043E952 FILE
// size      1218 bytes
// prototype void (__thiscall ?setup_veh@DesignWin@@QAEXHH@Z)(DesignWin* this, int protoID, int)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057E040 0x0057EFA0 0x00581260 0x005A63D0 0x005B9F20 0x00645460 0x00645660
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043E490
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043e490/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?setup_veh@DesignWin@@QAEXHH@Z  at 0x0043E490  (1218 bytes)
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

class DesignWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
bool __cdecl has_tech(int, int);
extern "C" char *strcpy(char *, const char *);
extern "C" int strcmp(const char *, const char *);
int __cdecl best_reactor(int);
void __cdecl consider_designs(int);
void __cdecl name_proto(char *, int, int, int, int, int, int, int);
void __cdecl proto_sort_2(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00733980 = (int *)0x00733980;
static int *const g_00733984 = (int *)0x00733984;
static int *const g_00733988 = (int *)0x00733988;
static int *const g_0093fc34 = (int *)0x0093FC34;
static int *const g_0093fc38 = (int *)0x0093FC38;
static int *const g_0093fc3c = (int *)0x0093FC3C;
static int *const g_0093fc40 = (int *)0x0093FC40;
static int *const g_0093fc50 = (int *)0x0093FC50;
static int *const g_0093fc54 = (int *)0x0093FC54;
static int *const g_00945b40 = (int *)0x00945B40;
static int *const g_00945b44 = (int *)0x00945B44;
static int *const g_009ab898 = (int *)0x009AB898;

class DesignWin { public:
    void setup_veh(int, int);
};

extern uint8_t g_ab898[];
extern uint8_t g_ab88c[];
extern uint8_t g_ab88d[];
extern uint8_t g_ab88e[];
extern uint8_t g_ab88f[];
extern int g_ab888[];
extern char g_ab868[];
extern short g_a3be[];
extern char g_a379[];
extern char g_a378[];
extern short g_ae6c[];
extern char g_ae68[];
extern short g_f284[];
extern char g_f280[];

void DesignWin::setup_veh(int a1, int a2) {
    int iVar7;

    if (a2 < 0 || (g_ab898[a2 * 0x1a] & 1) == 0 ||
        (a2 < 0x40 && (((signed char)g_ae68[(unsigned char)g_ab88d[a2 * 0x34] * 0x10] < 0) || a2 == 0xf))) {
        proto_sort_2(*(int *)((char *)this + 0x141f4));
        iVar7 = 0;
        if (*g_00945b40 > 0) {
            int *p = g_00945b44;
            do {
                if ((g_ab898[*p * 0x1a] & 1) == 0) break;
                iVar7++;
                p++;
            } while (iVar7 < *g_00945b40);
        }
        if (iVar7 == *g_00945b40) {
            *(int *)((char *)this + 0x141f8) = *(int *)((char *)this + 0x141f4) << 6;
        } else {
            *(int *)((char *)this + 0x141f8) = g_00945b44[iVar7];
        }
    } else {
        *(int *)((char *)this + 0x141f8) = a2;
    }

    unsigned char bVar8 = 0;
    iVar7 = *(int *)((char *)this + 0x141f8);
    *(int *)((char *)this + 0x1420c) = best_reactor(*(int *)((char *)this + 0x141f4));
    int local_8 = iVar7 * 0x34;

    if ((g_ab898[iVar7 * 0x1a] & 1) == 0) {
        *(int *)((char *)this + 0x14204) = 0;
        *(int *)((char *)this + 0x14200) = 0;
        if (a1 == 0) {
            *(int *)((char *)this + 0x141fc) = 0;
            *(int *)((char *)this + 0x14208) = 0;
            *(int *)((char *)this + 0x1420c) = 1;
            *(int *)((char *)this + 0x14218) = -1;
            *(int *)((char *)this + 0x1421c) = -1;
            *(int *)((char *)this + 0x14210) = 0;
            return;
        }

        int iVar4;
        iVar7 = 0;
        *(int *)((char *)this + 0x14208) = 0;
        *(int *)((char *)this + 0x141fc) = 0;
        do {
            int rc = has_tech((short)g_a3be[*(int *)((char *)this + 0x141fc) * 0x48],
                               *(int *)((char *)this + 0x141f4));
            if (rc != 0) {
                int idx90 = *(int *)((char *)this + 0x141fc) * 0x90;
                if (g_a379[idx90] == 0 && bVar8 <= (unsigned char)g_a378[idx90]) {
                    iVar7 = *(int *)((char *)this + 0x141fc);
                    bVar8 = g_a378[idx90];
                }
            }
            iVar4 = *(int *)((char *)this + 0x141fc) + 1;
            *(int *)((char *)this + 0x141fc) = iVar4;
        } while (iVar4 < 9);
        *(int *)((char *)this + 0x141fc) = iVar7;

        char cVar6 = 0;
        iVar7 = 0;
        *(int *)((char *)this + 0x14200) = 0;
        do {
            int rc = has_tech((short)g_ae6c[*(int *)((char *)this + 0x14200) * 8],
                               *(int *)((char *)this + 0x141f4));
            if (rc != 0) {
                char c1 = g_ae68[*(int *)((char *)this + 0x14200) * 0x10];
                if (c1 < 'c' && cVar6 <= c1) {
                    iVar7 = *(int *)((char *)this + 0x14200);
                    cVar6 = c1;
                }
            }
            iVar4 = *(int *)((char *)this + 0x14200) + 1;
            *(int *)((char *)this + 0x14200) = iVar4;
        } while (iVar4 < 0x1a);
        *(int *)((char *)this + 0x14200) = iVar7;

        cVar6 = 0;
        iVar7 = 0;
        *(int *)((char *)this + 0x14204) = 0;
        do {
            int rc = has_tech((short)g_f284[*(int *)((char *)this + 0x14204) * 8],
                               *(int *)((char *)this + 0x141f4));
            if (rc != 0) {
                if (cVar6 <= (char)g_f280[*(int *)((char *)this + 0x14204) * 0x10]) {
                    iVar7 = *(int *)((char *)this + 0x14204);
                    cVar6 = (char)g_f280[*(int *)((char *)this + 0x14204) * 0x10];
                }
            }
            iVar4 = *(int *)((char *)this + 0x14204) + 1;
            *(int *)((char *)this + 0x14204) = iVar4;
        } while (iVar4 < 0xe);
        *(int *)((char *)this + 0x14204) = iVar7;

        strcpy((char *)((char *)this + 0x14220), (char *)g_00733988);
        *g_0093fc3c = *(int *)((char *)this + 0x14200);
        *g_0093fc50 = *(int *)((char *)this + 0x14204);
        *g_0093fc40 = *(int *)((char *)this + 0x141fc);
        *g_0093fc34 = *(int *)((char *)this + 0x14208);
        *g_0093fc54 = 1;
        *g_0093fc38 = -1;
        consider_designs(*(int *)((char *)this + 0x141f4));
        *(int *)((char *)this + 0x14200) = *g_0093fc3c;
        *(int *)((char *)this + 0x14204) = *g_0093fc50;
        *(int *)((char *)this + 0x141fc) = *g_0093fc40;
        *(int *)((char *)this + 0x14208) = *g_0093fc34;
        *(int *)((char *)this + 0x14218) = -1;
        *(int *)((char *)this + 0x1421c) = -1;

        iVar7 = 0;
        for (;;) {
            if ((*(unsigned int *)((char *)this + 0x14208) & (1u << (iVar7 & 0x1f))) != 0) {
                int j = 0;
                int *p = (int *)((char *)this + 0x14218);
                do {
                    if (*p < 0) {
                        *(int *)((char *)this + 0x14218 + j * 4) = iVar7;
                        break;
                    }
                    j++;
                    p++;
                } while (j < 2);
            }
            iVar7++;
            if (iVar7 > 0x1c) {
                *g_0093fc54 = 0;
                g_ab898[local_8] = g_ab898[local_8] & 0x7f;
                *(int *)((char *)this + 0x14210) = 0;
                return;
            }
        }
    }

    *(unsigned short *)&g_ab898[iVar7 * 0x1a] &= 0xff7f;
    *(unsigned int *)((char *)this + 0x141fc) = g_ab88c[local_8];
    *(unsigned int *)((char *)this + 0x14200) = g_ab88d[local_8];
    *(unsigned int *)((char *)this + 0x14204) = g_ab88e[local_8];
    *(int *)((char *)this + 0x14208) = g_ab888[iVar7 * 0xd];
    *(unsigned int *)((char *)this + 0x1420c) = g_ab88f[local_8];
    *(int *)((char *)this + 0x14218) = -1;
    *(int *)((char *)this + 0x1421c) = -1;

    int iVar4b = 0;
    for (;;) {
        if ((*(unsigned int *)((char *)this + 0x14208) & (1u << (iVar4b & 0x1f))) != 0) {
            int j = 0;
            int *p = (int *)((char *)this + 0x14218);
            do {
                if (*p < 0) {
                    *(int *)((char *)this + 0x14218 + j * 4) = iVar4b;
                    break;
                }
                j++;
                p++;
            } while (j < 2);
        }
        iVar4b++;
        if (iVar4b > 0x1c) {
            if ((*(unsigned short *)&g_ab898[iVar7 * 0x1a] & 2) != 0 && iVar7 > 0x3f) {
                char *dst = (char *)((char *)this + 0x14220);
                strcpy(dst, (char *)g_ab868 + local_8);
                char local_58[80];
                name_proto(local_58, iVar7, *(int *)((char *)this + 0x141f4),
                           *(int *)((char *)this + 0x141fc), *(int *)((char *)this + 0x14200),
                           *(int *)((char *)this + 0x14204), *(int *)((char *)this + 0x14208),
                           *(int *)((char *)this + 0x1420c));
                int cmp = strcmp(dst, local_58);
                if (cmp == 0) {
                    strcpy(dst, (char *)g_00733980);
                    g_ab898[iVar7 * 0x1a] = g_ab898[iVar7 * 0x1a] & 0xfd;
                }
                *(int *)((char *)this + 0x14210) = 0;
                return;
            }
            *(unsigned short *)&g_ab898[iVar7 * 0x1a] &= 0xfffd;
            strcpy((char *)((char *)this + 0x14220), (char *)g_00733984);
            *(int *)((char *)this + 0x14210) = 0;
            return;
        }
    }
}
