// ORIGINAL: 0x00476C90 FILE
// name      ?mon_tech_discovered@@YAXHH@Z
// size      259 bytes
// spans     0x00476C90-0x00476D93
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00476A50 0x005C89A0
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00476C90
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00476c90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_tech_discovered@@YAXHH@Z  at 0x00476C90  (259 bytes)
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
int __cdecl game_year(int);
void __cdecl monument(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0094ca50 = (int *)0x0094CA50;
static int *const g_0094ca54 = (int *)0x0094CA54;
static int *const g_0094f1f0 = (int *)0x0094F1F0;
static int *const g_0094f1f4 = (int *)0x0094F1F4;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_0094ca40 = (int *)0x0094CA40;
static int *const g_0094ca48 = (int *)0x0094CA48;
static int *const g_0094ca4c = (int *)0x0094CA4C;
static int *const g_0094cea0 = (int *)0x0094CEA0;

void __cdecl mon_tech_discovered(int a1, int a2) {
    int year = game_year(*g_009a64d4);

    int record_offset = a1 * 0x4f4;
    int flag_offset = a1 * 0x59c;

    if (*reinterpret_cast<unsigned char *>(
            reinterpret_cast<char *>(g_00946f58) + flag_offset) & 0x80) {
        return;
    }
    if (*reinterpret_cast<int *>(
            reinterpret_cast<char *>(g_0094ca50) + record_offset) != 0) {
        return;
    }

    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(g_0094ca54) + record_offset) = 0;
    int i = 0;
    int p = reinterpret_cast<int>(g_0094ca50);
    do {
        if (i != a1 && *reinterpret_cast<int *>(p) != 0) {
            *reinterpret_cast<int *>(
                reinterpret_cast<char *>(g_0094ca54) + record_offset) += 1;
        }
        p += 0x4f4;
        i++;
    } while (p < reinterpret_cast<int>(g_0094f1f0));

    int counter = *reinterpret_cast<int *>(
        reinterpret_cast<char *>(g_0094ca54) + record_offset);
    int chosen;
    if (counter != 0) {
        *reinterpret_cast<int *>(
            reinterpret_cast<char *>(g_0094ca54) + record_offset) = 0;
        chosen = a1;
        int j = 0;
        int q = reinterpret_cast<int>(g_0094ca54);
        do {
            if (j != a1 && *reinterpret_cast<int *>(q - 4) != 0 &&
                *reinterpret_cast<int *>(q) != 0) {
                chosen = j;
            }
            q += 0x4f4;
            j++;
        } while (q < reinterpret_cast<int>(g_0094f1f4));
    } else {
        *reinterpret_cast<int *>(
            reinterpret_cast<char *>(g_0094ca54) + record_offset) = 1;
        chosen = a1;
    }

    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(g_0094ca50) + record_offset) = 1;
    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(g_0094ca40) + record_offset) = year;
    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(g_0094ca4c) + record_offset) = chosen;
    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(g_0094ca48) + record_offset) = a2;
    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(g_0094cea0) + record_offset) = 1;

    if (a1 == *g_00939284) {
        monument(1);
    }
}

