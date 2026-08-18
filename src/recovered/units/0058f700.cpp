// ORIGINAL: 0x0058F700 ?say_fac_special@@YAXPADPADH@Z 0x0058F700-0x0058F80A FILE BYTE_EXACT
// size      266 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00625880 0x00645470
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0058F700
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0058f700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_fac_special@@YAXPADPADH@Z  at 0x0058F700  (266 bytes)
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
// Declared under its decorated name, not `strcat`: CL recognises the CRT
// name as an intrinsic at /O2 and inlines the byte-scan expansion in place
// of a call, which the original does not do (it calls out to 0x645470).
extern "C" char *__cdecl _strcat(char *, const char *);
int __cdecl parse_string(char *, char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068fab0 = (int *)0x0068FAB0;
static int *const g_0068fab4 = (int *)0x0068FAB4;
static int *const g_0068fab8 = (int *)0x0068FAB8;
static int *const g_0068fabc = (int *)0x0068FABC;
static int *const g_0068fac0 = (int *)0x0068FAC0;
static int *const g_0068fac4 = (int *)0x0068FAC4;
static int *const g_0068fac8 = (int *)0x0068FAC8;
static int *const g_0068facc = (int *)0x0068FACC;

// Indexed table bases: the address itself is added to a computed index, so
// the const-pointer spelling (which /O2 folds into a literal and loses the
// add) is wrong here - see "the lever that keeps working" above.
extern int g_00946d4c[];
extern int g_00946d50[];

void __cdecl say_fac_special(char *a1, char *a2, int a3) {
    char buf1[256];
    char buf2[256];

    int offset = a3 * 0x59c;
    int table_b = *reinterpret_cast<int *>(
        reinterpret_cast<char *>(g_00946d50) + offset);
    int table_a = *reinterpret_cast<int *>(
        reinterpret_cast<char *>(g_00946d4c) + offset);

    buf1[0] = 0;
    _strcat(buf1, reinterpret_cast<char *>(g_0068fab0));

    if (table_a == 0) {
        _strcat(buf1, reinterpret_cast<char *>(g_0068fab4));
    } else if (table_a == 1) {
        _strcat(buf1, reinterpret_cast<char *>(g_0068fab8));
    } else if (table_a == 2) {
        _strcat(buf1, reinterpret_cast<char *>(g_0068fabc));
    }

    if (table_b == 0) {
        _strcat(buf1, reinterpret_cast<char *>(g_0068fac0));
    } else if (table_b == 1) {
        _strcat(buf1, reinterpret_cast<char *>(g_0068fac4));
    }

    _strcat(buf1, reinterpret_cast<char *>(g_0068fac8));
    _strcat(buf1, a2);
    _strcat(buf1, reinterpret_cast<char *>(g_0068facc));

    parse_string(buf1, buf2);
    _strcat(a1, buf2);
}

