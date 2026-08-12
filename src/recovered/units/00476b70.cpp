// ORIGINAL: 0x00476B70 FILE
// name      ?mon_colony_founded@@YAXHPAD@Z
// size      276 bytes
// spans     0x00476B70-0x00476C84
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00476A50 0x005C89A0 0x00645E90
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00476B70
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00476b70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_colony_founded@@YAXHPAD@Z  at 0x00476B70  (276 bytes)
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
extern "C" char * __cdecl strncpy(char *, const char *, unsigned int);
int __cdecl game_year(int);
void __cdecl monument(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
// Table base: the address itself is indexed (`[reg*4 + base]`), so a
// folded literal would lose the addressing mode. Declared as a byte array.
extern uint8_t g_00946f58[];
static int *const g_0094ca18 = (int *)0x0094CA18;
static int *const g_0094ca1c = (int *)0x0094CA1C;
static int *const g_0094f1b8 = (int *)0x0094F1B8;
static int *const g_0094f1bc = (int *)0x0094F1BC;
static int *const g_009a64d4 = (int *)0x009A64D4;

void __cdecl mon_colony_founded(int a1, char * a2) {
    int founded_year = game_year(*g_009a64d4);
    int rec_off = a1 * 0x4F4;

    if ((g_00946f58[a1 * 0x59C] & 0x80) == 0 &&
        *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0094ca18) + rec_off) == 0) {
        *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0094ca1c) + rec_off) = 0;

        int i = 0;
        int p = reinterpret_cast<int>(g_0094ca18);
        do {
            if (i != a1 && *reinterpret_cast<int *>(p) != 0) {
                ++*reinterpret_cast<int *>(reinterpret_cast<char *>(g_0094ca1c) + rec_off);
            }
            p += 0x4F4;
            ++i;
        } while (p < reinterpret_cast<int>(g_0094f1b8));

        int last;
        if (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_0094ca1c) + rec_off) != 0) {
            *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0094ca1c) + rec_off) = 0;
            last = a1;

            int j = 0;
            int q = reinterpret_cast<int>(g_0094ca1c);
            do {
                if (j != a1 && *reinterpret_cast<int *>(q - 4) != 0 &&
                    *reinterpret_cast<int *>(q) != 0) {
                    last = j;
                }
                q += 0x4F4;
                ++j;
            } while (q < reinterpret_cast<int>(g_0094f1bc));
        } else {
            *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0094ca1c) + rec_off) = 1;
            last = a1;
        }

        *reinterpret_cast<int *>(0x0094CA14 + rec_off) = last;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0094ca18) + rec_off) = 1;
        *reinterpret_cast<int *>(0x0094CA08 + rec_off) = founded_year;
        strncpy(reinterpret_cast<char *>(0x0094CEB4 + rec_off), a2, 0x18);

        int cur = *g_00939284;
        *reinterpret_cast<int *>(0x0094CEA0 + rec_off) = 0;
        if (a1 == cur) {
            monument(1);
        }
    }
}
