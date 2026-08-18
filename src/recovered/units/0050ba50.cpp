// ORIGINAL: 0x0050BA50 ?intervention@@YAXHH@Z 0x0050BA50-0x0050BCB6 FILE
// size      614 bytes
// prototype 
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0055B870 0x0055BB30 0x005BF310 0x00625EC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0050BA50
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0050ba50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?intervention@@YAXHH@Z  at 0x0050BA50  (614 bytes)
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
int __cdecl X_pop(const char *, int (__cdecl *)());
int __cdecl parse_says(int, char *, int, int);
void __cdecl set_treaty(int, int, int, int);
void __cdecl treaty_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068a308 = (int *)0x0068A308;
static int *const g_0068a314 = (int *)0x0068A314;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009472ec = (int *)0x009472EC;
static int *const g_00949a30 = (int *)0x00949A30;
static int *const g_0096ca58 = (int *)0x0096CA58;
static int *const g_0096eac4 = (int *)0x0096EAC4;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a64e9 = (int *)0x009A64E9;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// Indexed table base: 0x96eac4 is indexed by a register (a1/a2) with a
// scale-4 lea, so the address itself does work - needs `extern T name[]`
// under a name distinct from the context's own const-pointer global. Two
// distinct symbols (rather than one indexed twice) because collapsing them
// to a single name invites the optimizer to represent the second pointer as
// a delta off the first - see CHANGE in the recovery report.
extern int g_table_96eac4_a[];
extern int g_table_96eac4_b[];
// Same story for the per-faction record tables: base + reg*0x59c (a
// strength-reduced multiply), never a plain load/store of the base itself.
extern unsigned char g_table_946d34[];
extern unsigned char g_table_946a50[];
// 0x96ca58 is stored to at `[ecx*4 + 0x96ca58]`, ecx computed from i/a2.
extern int g_table_96ca58[];

void __cdecl intervention(int a1, int a2) {
    int i = 1;
    int *ptrA = g_table_96eac4_a + a1;
    int *ptrB = g_table_96eac4_b + a2;
    int *p = g_009472ec;

    do {
        if (i != a1 && i != a2 &&
            (static_cast<unsigned char>(1 << i) &
             *reinterpret_cast<unsigned char *>(g_009a64e9)) != 0 &&
            (*reinterpret_cast<unsigned char *>(ptrA) & 1) != 0 &&
            (*reinterpret_cast<unsigned char *>(ptrB) & 0x11) == 0) {

            *g_009bbfec = p[-1];
            *g_009bbff0 = p[0];
            parse_says(0, reinterpret_cast<char *>(p - 7), -1, -1);

            int rowA = a1 * 0x59c;
            *g_009bbfec = *reinterpret_cast<int *>(g_table_946d34 + rowA + 0x18);
            *g_009bbff0 = *reinterpret_cast<int *>(g_table_946d34 + rowA + 0x1c);
            parse_says(1, reinterpret_cast<char *>(g_table_946d34 + rowA), -1, -1);

            *g_009bbfec = p[-0xC0];
            *g_009bbff0 = 0;
            parse_says(2, reinterpret_cast<char *>(p - 0xAD), -1, -1);

            *g_009bbfec = p[-0xC0];
            *g_009bbff0 = 0;
            parse_says(3, reinterpret_cast<char *>(p - 0xB3), -1, -1);

            int rowB = a2 * 0x59c;
            *g_009bbfec = *reinterpret_cast<int *>(g_table_946a50 + rowB);
            *g_009bbff0 = 0;
            parse_says(4, reinterpret_cast<char *>(g_table_946a50 + rowB + 0x4c), -1, -1);

            *g_009bbfec = *reinterpret_cast<int *>(g_table_946a50 + rowB);
            *g_009bbff0 = 0;
            parse_says(5, reinterpret_cast<char *>(g_table_946a50 + rowB + 0x34), -1, -1);

            *g_009bbfec = *reinterpret_cast<int *>(g_table_946d34 + rowB + 0x18);
            *g_009bbff0 = *reinterpret_cast<int *>(g_table_946d34 + rowB + 0x1c);
            parse_says(6, reinterpret_cast<char *>(g_table_946d34 + rowB), -1, -1);

            unsigned int mask = static_cast<unsigned int>(*g_009a64e8) & 0xff;
            unsigned int bitA2 = mask & (1u << a2);
            if (bitA2 != 0 || (mask & (1u << a1)) != 0) {
                if (bitA2 != 0) {
                    g_table_96ca58[i + a2 * 2099] = *g_009a64d4;
                }
                treaty_on(i, a2, 0x80000018);
                set_treaty(i, a2, 0x40, 1);
            }

            if (a2 == *g_00939284) {
                X_pop(reinterpret_cast<const char *>(g_0068a308), 0);
            } else if (a1 == *g_00939284) {
                X_pop(reinterpret_cast<const char *>(g_0068a314), 0);
            }
        }

        p += 359;
        ptrA += 2099;
        ptrB += 2099;
        i++;
    } while (p < g_00949a30);
}
