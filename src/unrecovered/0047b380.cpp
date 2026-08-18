// ORIGINAL: 0x0047B380 ?get_player_color@NetWin@@QAEHH@Z 0x0047B380-0x0047B442 FILE
// size      194 bytes
// prototype int (__thiscall ?get_player_color@NetWin@@QAEHH@Z)(NetWin* this, int)
// callers   4   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_player_color@NetWin@@QAEHH@Z  at 0x0047B380  (194 bytes)
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

class NetWin;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090db9b = (int *)0x0090DB9B;
static int *const g_0090dd17 = (int *)0x0090DD17;
static int *const g_0093d4f8 = (int *)0x0093D4F8;

// INDEXED TABLE BASE lever: both tables are stepped with the address baked
// into the addressing mode (`[eax*4+0x90db9b]`, and a pointer walk starting
// at `&DAT_0090dd17`), so own externs are used instead of the folded-literal
// globals above.
extern int8_t g_table_db9b[];
extern int8_t g_table_dd17[];

class NetWin { public:
    int get_player_color(int);
};
int NetWin::get_player_color(int a1) {
    // RULED-OUT: 194/213 bytes (rebuilt is LARGER, first divergence in the
    // prologue). Original does `push ebp; mov ebp,esp; push ecx` - the
    // `push ecx` is a 1-byte "reserve one local dword" idiom, and that slot
    // is then overwritten with the `this->+0x772c` pointer value and RELOADED
    // from `[ebp-4]` partway through the second search loop; esi/edi are
    // pushed unconditionally up front too. This function's register/stack
    // pressure across the two nested linear scans is high enough that VC6
    // spills that pointer to a dedicated frame slot; every source form
    // tried here - caching it in one local, not caching it at all and
    // re-reading `*(int*)(self+0x772c)` at each use site, a `for(;;)`
    // rewrite of the trailing loop instead of the comma-expression
    // `while` - keeps it in registers instead and ends up computing MORE
    // instructions overall to make up the difference. Not found in ~4 tries.
    char *self = reinterpret_cast<char *>(this);
    int obj = *reinterpret_cast<int *>(self + 0x772c);
    if (obj == 0) {
        return 0;
    }
    int color = g_table_db9b[a1 * 0x17c];
    if (color == -1) {
        return 0xff;
    }
    int k = 1;
    if (a1 > 1) {
        char *p = reinterpret_cast<char *>(g_table_dd17);
        do {
            if (*p == color) break;
            k++;
            p += 0x17c;
        } while (k < a1);
    }
    int index = 0;
    if (k != a1) {
        index = 0;
        int playerCount = *reinterpret_cast<uint8_t *>(obj + 1);
        if (playerCount != 0) {
            int bound = *g_0093d4f8 + 1;
            for (;;) {
                int found = 1;
                if (bound > 1) {
                    char *p = reinterpret_cast<char *>(g_table_dd17);
                    do {
                        if (*p == index) break;
                        found++;
                        p += 0x17c;
                    } while (found < bound);
                }
                if (found == bound) {
                    break;
                }
                index++;
                if (index >= playerCount) {
                    break;
                }
            }
        }
    }
    return *reinterpret_cast<uint8_t *>(obj + 8 + index * 400);
}
