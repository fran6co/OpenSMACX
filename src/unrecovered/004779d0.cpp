// ORIGINAL: 0x004779D0 ?mon_winning_trans@@YAXHH@Z 0x004779D0-0x00477A57 FILE
// TRIED: extern "C" on the already-declared (plain-linkage) game_year / monument callees - conflicts as C2732. Declaring a new byte table extern for 0x94cd18 (not auto-picked-up by the emitter, only ever seen via [ecx+const] addressing) compiles. MISMATCH #11 'lea' vs 'sub' remains open.
// working copy - scaffold materialised by --work
// size      135 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00476A50 0x005C89A0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_winning_trans@@YAXHH@Z  at 0x004779D0  (135 bytes)
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
int game_year(int);
void monument(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009a64d4 = (int *)0x009A64D4;
extern unsigned char g_rectable_94cd18[];

void __cdecl mon_winning_trans(int a1, int a2) {
    int yearVal = game_year(*g_009a64d4);

    unsigned char *flagRec = reinterpret_cast<unsigned char *>(g_00946f58) + (unsigned)a1 * 0x59c;
    if (flagRec[0] & 0x80) {
        return;
    }

    unsigned char *rec = g_rectable_94cd18 + (unsigned)a1 * 0x4f4;
    if (*reinterpret_cast<int *>(rec + 0x10) != 0) {
        return;
    }

    *reinterpret_cast<int *>(rec + 0x00) = yearVal;
    *reinterpret_cast<int *>(rec + 0x0c) = a1;
    *reinterpret_cast<int *>(rec + 0x14) = 1;
    *reinterpret_cast<int *>(rec + 0x10) = 1;
    *reinterpret_cast<int *>(rec + 0x08) = a2;

    *reinterpret_cast<int *>(rec + 0x188) = 0xe;
    if (a1 == *g_00939284) {
        monument(1);
    }
}
