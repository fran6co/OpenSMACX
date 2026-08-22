// ORIGINAL: 0x004CDA30 ?action_patrol@@YAXHHH@Z 0x004CDA30-0x004CDAEA FILE
// TRIED: fixed const-pointer globals for the two indexed word arrays (address itself scales, so `extern`-style raw casts used instead). Reached MNEMONIC_ONLY (188 vs 186 B).
// working copy - scaffold materialised by --work
// size      186 bytes
// prototype void (__cdecl ?action_patrol@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004CD6A0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action_patrol@@YAXHHH@Z  at 0x004CDA30  (186 bytes)
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
int valid_patrol(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0095283c = (int *)0x0095283C;
static int *const g_00952844 = (int *)0x00952844;
void __cdecl action_patrol(int a1, int a2, int a3) {
    if (valid_patrol(a1, a2, a3) != 0) {
        int idx34 = a1 * 0x34;
        int idx1a = a1 * 0x1a;
        // Indexed table bases; the address itself does work (scaled
        // index), so plain casts are used instead of the fixed globals.
        unsigned char *arr_952839 = reinterpret_cast<unsigned char *>(0x00952839);
        unsigned char *arr_95283a = reinterpret_cast<unsigned char *>(0x0095283A);
        unsigned char *arr_95283b = reinterpret_cast<unsigned char *>(0x0095283B);
        short *arr_95283c = reinterpret_cast<short *>(0x0095283C);
        short *arr_952844 = reinterpret_cast<short *>(0x00952844);
        short *arr_952828 = reinterpret_cast<short *>(0x00952828);
        short *arr_95282a = reinterpret_cast<short *>(0x0095282A);
        int *arr_95282c = reinterpret_cast<int *>(0x0095282C);

        arr_952839[idx34] = 0x18;
        arr_95283c[(unsigned char)arr_95283a[idx34] + idx1a] = (short)a2;
        arr_952844[(unsigned char)arr_95283a[idx34] + idx1a] = (short)a3;
        unsigned char cur = arr_95283a[idx34];
        arr_95283a[idx34] = cur + 1;
        arr_95283c[(unsigned char)(cur + 1) + idx1a] = arr_952828[idx1a];
        arr_952844[(unsigned char)arr_95283a[idx34] + idx1a] = arr_95282a[idx1a];
        arr_95282c[a1 * 0xd] |= 0x1000200;
        arr_95283b[idx34] = 0;
    }
}
