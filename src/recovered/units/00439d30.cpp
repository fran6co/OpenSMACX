// ORIGINAL: 0x00439D30 ?suggest_abil@DesignWin@@QAEXH@Z 0x00439D30-0x00439EA3 FILE
// size      371 bytes
// prototype void (__thiscall ?suggest_abil@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   3   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00439C00 0x0050BA30 0x00581170 0x00584B50 0x005B9F20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00439D30
// measured tier  MISMATCH
// divergence     69
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439d30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?suggest_abil@DesignWin@@QAEXH@Z  at 0x00439D30  (371 bytes)
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
int __cdecl abil_cond(int, int, int, int);
int __cdecl abil_index(int);
int __cdecl bit_count(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f60 = (int *)0x00946F60;
static int *const g_00946f80 = (int *)0x00946F80;
static int *const g_00949780 = (int *)0x00949780;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009ab550 = (int *)0x009AB550;

class DesignWin { public:
    void check_abil();
    void suggest_abil(int);
};

// Indexed table bases need `extern T name[]`, not the const-pointer form -
// the emitter cannot detect base-immediate/offset-register on its own. The
// scaffold already binds these addresses as `static int *const`, so a
// distinct name is used for the array form (same relocation, same bytes;
// precedent: build/byte-match/0043be30's g_009ab898_tbl).
extern int g_00946f60_tbl[];
extern int g_00946f80_tbl[];
extern short g_009ab550_tbl[];

// MISMATCH (#69: original 'lea' vs rebuilt 'push'/'mov', tried four source
// forms - a named `row2` local before or after the `abil_index(a1)` call,
// and the comparison inlined both ways round - all four land on the exact
// same instruction index with a different mnemonic each time. Everything
// through index 68 (every call, every branch, the `i` vs `row` init order)
// is byte-exact; this is REGISTER ALLOCATION / SCHEDULING for the second,
// redundant `g_939284 * 359` recompute inside the loop body, not a spelling
// this agent found.
void DesignWin::suggest_abil(int a1) {
    char *self = reinterpret_cast<char *>(this);
    check_abil();
    if (*g_009a6490 & 0x4000) {
        int *f141f4 = reinterpret_cast<int *>(self + 0x141f4);
        int *f141fc = reinterpret_cast<int *>(self + 0x141fc);
        int *f14200 = reinterpret_cast<int *>(self + 0x14200);
        int *f14204 = reinterpret_cast<int *>(self + 0x14204);
        int *f14208 = reinterpret_cast<int *>(self + 0x14208);
        int *f14218 = reinterpret_cast<int *>(self + 0x14218);
        if (has_tech(g_009ab550_tbl[abil_index(a1) * 14], *f141f4)) {
            if (abil_cond(abil_index(a1), *f141fc, *f14200, *f14204)) {
                if (bit_count(*f14208) <= 1) {
                    if (bit_count(*f14208) == 0 || has_tech(*g_00949780, *f141f4)) {
                        int i = 0;
                        int row = *g_00939284 * 359;
                        for (; i < 0x29; i++) {
                            if (g_00946f60_tbl[row + i] == 15) {
                                if (g_00946f80_tbl[(*g_00939284 * 359) + i] == abil_index(a1)) return;
                            }
                        }
                        *f14208 |= a1;
                        f14218[0] = -1;
                        f14218[1] = -1;
                        for (int j = 0; j < 0x1d; j++) {
                            if (*f14208 & (1 << j)) {
                                for (int k = 0; k < 2; k++) {
                                    if (f14218[k] < 0) {
                                        f14218[k] = j;
                                        break;
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
