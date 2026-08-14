// ORIGINAL: 0x0048CF50 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      ?veh_sprite_click@@YAHH@Z
// size      182 bytes
// spans     0x0048CF50-0x0048D006
// prototype 
// callers   0   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00531020 0x005310F0 0x00531480 0x00593220 0x005C1A20 0x005C1C40 0x005C1D70

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?veh_sprite_click@@YAHH@Z  at 0x0048CF50  (182 bytes)
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

class NetDaemon;
class NetMsg;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int lock_veh(int *, int, int, int, int);
    void await_synch();
    void unlock_veh();
};

class NetMsg { public:
    void pop(const char *, int, int, const char *);
};

int veh_jail(int);
int veh_selectable(int);
int veh_wake(int);
void synch_veh(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00686b5c = (int *)0x00686B5C;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
int __cdecl veh_sprite_click(int a1) {
    if (*g_0093f660 != 0 && veh_selectable(a1) != 0 &&
        *((char *)0x00952839 + a1 * 0x34) == 0 &&
        (*(int *)((char *)0x0095282c + a1 * 0x34) & 0x8004200) == 0) {
        return 1;
    }
    if (veh_jail(a1) != 0) {
        ((NetMsg *)g_00805338)->pop((const char *)g_00686b5c, 5000, 0, 0);
        return 0;
    }
    if (((NetDaemon *)g_0093cd90)->lock_veh(&a1, 0, -1, -1, 0) == 0) {
        veh_wake(a1);
        synch_veh(a1);
        ((NetDaemon *)g_0093cd90)->await_synch();
        ((NetDaemon *)g_0093cd90)->unlock_veh();
    }
    return 0;
}
