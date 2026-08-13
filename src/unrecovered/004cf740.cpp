// ORIGINAL: 0x004CF740 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      ?action@@YAHH@Z
// size      116 bytes
// spans     0x004CF740-0x004CF7B4
// prototype int (__cdecl ?action@@YAHH@Z)(int vehID)
// callers   3   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C9B00 0x004CB310 0x004CB580

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action@@YAHH@Z  at 0x004CF740  (116 bytes)
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
int action_terraform(int, int, int);
void action_go_to(int);
void action_road_to(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00952839 = (int *)0x00952839;
int __cdecl action(int a1) {
    char *table = reinterpret_cast<char *>(g_00952839);
    char state = table[a1 * 0x34];
    if (state >= 4 && state < 0x18) {
        action_terraform(a1, state - 4, 1);
    } else if (state == 0x1b || state == 0x1c) {
        action_road_to(a1);
    } else if (state == 0x18 || state == 0x58 || state == 0x19) {
        action_go_to(a1);
    } else {
        return 0;
    }
    if (table[a1 * 0x34] & 0x40) {
        table[a1 * 0x34] = 0;
    }
    return 1;
}
