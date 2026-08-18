// ORIGINAL: 0x004CF480 ?action_sat_attack@@YAXHHHH@Z 0x004CF480-0x004CF73E FILE
// size      702 bytes
// prototype void (__cdecl ?action_sat_attack@@YAXHHHH@Z)(int factionIDAtk, int factionIDDef, int type, int baseID)
// callers   3   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0048C0A0 0x004A3FC0 0x004E48B0 0x0050BCC0 0x00625E50 0x00625EC0 0x0064601D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004CF480
// measured tier  MISMATCH
// divergence     25
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004cf480/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action_sat_attack@@YAXHHHH@Z  at 0x004CF480  (702 bytes)
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

class ReportIf;

// ---- callees, declared and never defined (a definition would be inlined) ----
class ReportIf { public:
    void on_redraw();
};
extern "C" int rand();
int __cdecl parse_say(int, int, int, int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
void __cdecl double_cross(int, int, int);
void __cdecl set_fac(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006882d0 = (int *)0x006882D0;
static int *const g_006882e0 = (int *)0x006882E0;
static int *const g_006882ec = (int *)0x006882EC;
static int *const g_006882fc = (int *)0x006882FC;
static int *const g_0068830c = (int *)0x0068830C;
static int *const g_0068831c = (int *)0x0068831C;
static int *const g_00688328 = (int *)0x00688328;
static int *const g_00688338 = (int *)0x00688338;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_007ae778 = (int *)0x007AE778;
static int *const g_007d392c = (int *)0x007D392C;
static int *const g_00885f38 = (int *)0x00885F38;
static int *const g_00885f40 = (int *)0x00885F40;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096cd44 = (int *)0x0096CD44;
static int *const g_0096d1e8 = (int *)0x0096D1E8;
static int *const g_0096d1f4 = (int *)0x0096D1F4;
static int *const g_009a5318 = (int *)0x009A5318;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// INDEXED TABLE BASE lever: every one of these is indexed by a register
// computed from the parameters, so each needs array linkage rather than
// the emitter's plain pointer form (which would fold the base address
// away under /O2 and drop the addressing entirely).
extern unsigned char g_0096c9f8_arr[];
extern char g_00946a50_arr[];
extern int g_0096d1e8_tbl[];
extern int g_0096d1f4_tbl[];
extern int g_0096cd44_tbl[];
extern int g_009a5798_tbl[];
extern int g_007ae778_tbl[];

void __cdecl action_sat_attack(int a1, int a2, int a3, int a4) {
    if ((g_0096c9f8_arr[a2 * 4 + a1 * 0x20cc] & 0x10) == 0) {
        double_cross(a1, a2, -1);
    }

    if (a1 == *g_00939284 || a2 == *g_00939284) {
        char *rec1 = g_00946a50_arr + a1 * 0x59c;

        *g_009bbff0 = 0;
        *g_009bbfec = *reinterpret_cast<int *>(rec1);
        parse_says(0, rec1 + 0x4c, -1, -1);

        *g_009bbfec = *reinterpret_cast<int *>(rec1);
        *g_009bbff0 = 0;
        parse_says(1, rec1 + 0x34, -1, -1);

        *g_009bbfec = *reinterpret_cast<int *>(rec1 + 0x2fc);
        *g_009bbff0 = *reinterpret_cast<int *>(rec1 + 0x300);
        parse_says(2, rec1 + 0x2e4, -1, -1);

        char *rec2 = g_00946a50_arr + a2 * 0x59c;

        *g_009bbff0 = 0;
        *g_009bbfec = *reinterpret_cast<int *>(rec2);
        parse_says(3, rec2 + 0x4c, -1, -1);

        *g_009bbfec = *reinterpret_cast<int *>(rec2);
        *g_009bbff0 = 0;
        parse_says(4, rec2 + 0x34, -1, -1);

        *g_009bbfec = *reinterpret_cast<int *>(rec2 + 0x2fc);
        *g_009bbff0 = *reinterpret_cast<int *>(rec2 + 0x300);
        parse_says(5, rec2 + 0x2e4, -1, -1);

        int sayVal = (a3 == 4) ? *g_009a5318 : g_009a5798_tbl[a3 * 12];
        parse_say(6, sayVal, -1, -1);

        parse_says(7, rec2 + 0x304, -1, -1);
    }

    if (rand() % 100 < 50) {
        if (a3 == 4) {
            set_fac(0x29, a4, 0);
        } else {
            g_0096d1e8_tbl[a2 * 0x833 + a3]--;
        }
        g_0096cd44_tbl[a1 * 0x833]++;

        if (a1 == *g_00939284) {
            popp(*reinterpret_cast<char **>(g_00691b0c),
                 reinterpret_cast<char *>(g_006882e0), 0,
                 reinterpret_cast<char *>(g_006882d0), 0);
        } else if (a2 == *g_00939284) {
            popp(*reinterpret_cast<char **>(g_00691b0c),
                 reinterpret_cast<char *>(g_006882fc), 0,
                 reinterpret_cast<char *>(g_006882ec), 0);
        }
    } else {
        g_0096d1f4_tbl[a1 * 0x833]--;

        if (a1 == *g_00939284) {
            popp(*reinterpret_cast<char **>(g_00691b0c),
                 reinterpret_cast<char *>(g_0068831c), 0,
                 reinterpret_cast<char *>(g_0068830c), 0);
        } else if (a2 == *g_00939284) {
            popp(*reinterpret_cast<char **>(g_00691b0c),
                 reinterpret_cast<char *>(g_00688338), 0,
                 reinterpret_cast<char *>(g_00688328), 0);
        }
    }

    if (*g_00885f40 == 6 && g_007ae778_tbl[*g_007d392c] == 5) {
        reinterpret_cast<ReportIf *>(g_00885f38)->on_redraw();
    }
}
