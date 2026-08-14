// ORIGINAL: 0x0051F200 FILE
// working copy - scaffold materialised by --work
// name      ?wait_loop@@YAXXZ
// size      191 bytes
// spans     0x0051F200-0x0051F2BF
// prototype 
// callers   7   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00530360 0x005304A0 0x0054FFD0 0x006262F0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wait_loop@@YAXXZ  at 0x0051F200  (191 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int open_channel(int, int);
    void net_tasks();
};

void communicate(int, int, int);
void log_say(char *, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068b064 = (int *)0x0068B064;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a934 = (int *)0x0093A934;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8ec = (int *)0x0093E8EC;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f67c = (int *)0x0093F67C;
static int *const g_0093f698 = (int *)0x0093F698;
static int *const g_009a64e8 = (int *)0x009A64E8;
void __cdecl wait_task();

void __cdecl wait_loop() {
    if (*g_0093f660 != 0) {
        ((NetDaemon *)g_0093cd90)->net_tasks();
        if (*g_0093a934 == 0 && *g_0093a938 == 0 &&
            (*g_0093e8ec & (1 << *g_00939284)) == 0) {
            int *slot = g_0093f67c;
            for (int i = 1; slot < g_0093f698; ++i, ++slot) {
                if (i != *g_00939284 &&
                    (*(char *)g_009a64e8 & (char)(1 << i)) == 0 &&
                    *slot != 0) {
                    log_say((char *)g_0068b064, *g_00939284, i, 0);
                    if (((NetDaemon *)g_0093cd90)->open_channel(*g_00939284, i) == 0) {
                        communicate(*g_00939284, i, 0);
                    }
                }
            }
        }
    } else {
        wait_task();
    }
}
