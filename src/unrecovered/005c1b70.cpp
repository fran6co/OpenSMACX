// ORIGINAL: 0x005C1B70 ?veh_ready@@YAHH@Z 0x005C1B70-0x005C1C38 FILE
// RULED-OUT: separate 'if (cond) return 0;' per check duplicated the epilogue (245 vs 200 bytes); combined into nested && conditions sharing one trailing `return 0` (lever: single-exit chain). extern char[] base for the runtime a1*0x34 struct index. MISMATCH from #9: the a1*0x34 multiply still lowers via a different instruction chain than the original's lea/lea/shl
// working copy - scaffold materialised by --work
// size      200 bytes
// prototype int (__cdecl ?veh_ready@@YAHH@Z)(int vehID)
// callers   14   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540 0x005C1C40

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?veh_ready@@YAHH@Z  at 0x005C1B70  (200 bytes)
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
int speed(int, int);
int veh_jail(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
extern char g_952828_arr[];

int __cdecl veh_ready(int a1) {
    if (a1 >= 0) {
        char *entry = g_952828_arr + a1 * 0x34;
        short y = *reinterpret_cast<short *>(entry + 2);
        short x = *reinterpret_cast<short *>(entry + 0);
        if (y >= 0 && y < *g_00949874 && x >= 0 && x < *g_00949870) {
            unsigned char owner = *reinterpret_cast<unsigned char *>(entry + 0xe);
            unsigned short flags8 = *reinterpret_cast<unsigned short *>(entry + 8);
            if (static_cast<int>(owner) == *g_00939284 &&
                ((flags8 & 0x40) == 0 || (flags8 & 0x400) == 0)) {
                if (veh_jail(a1) == 0) {
                    int flags4 = *reinterpret_cast<int *>(entry + 4);
                    unsigned char stat = *reinterpret_cast<unsigned char *>(entry + 0x11);
                    if ((flags4 & 0x4200) != 0 ||
                        !(stat == 1 || stat == 2 || stat == 3)) {
                        int cur = speed(a1, 0);
                        unsigned char base = *reinterpret_cast<unsigned char *>(entry + 0x28);
                        int diff = cur - base;
                        if (diff >= 0 && (diff > 0x3e7 || diff != 0)) {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
