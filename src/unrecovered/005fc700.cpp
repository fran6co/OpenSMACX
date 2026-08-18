// ORIGINAL: 0x005FC700 ?wait_task@@YAXXZ 0x005FC700-0x005FC7E4 FILE
// RULED-OUT: 260 vs 228 bytes; VC6's /O2 loop rotation duplicates the do_video/check_net/do_net top-of-loop block into a separate call sequence instead of the original's fall-through-then-jmp reuse of one copy. Tried both `for (;;)` and `do {} while(1)` for the poll loop; both rotate the same way.
// working copy - scaffold materialised by --work
// size      228 bytes
// prototype 
// callers   15   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00632DB0 0x00636300
// indirect  0x005FC757 0x005FC762 0x005FC769 0x005FC791 0x005FC7A9 0x005FC7D4 0x005FC7DB

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wait_task@@YAXXZ  at 0x005FC700  (228 bytes)
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

class Net;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Net { public:
    void handle_sys_msg();
};

void check_net();
void do_net();
void do_video();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669340 = (int *)0x00669340;
static int *const g_00669358 = (int *)0x00669358;
static int *const g_0066935c = (int *)0x0066935C;
static int *const g_009b7b9c = (int *)0x009B7B9C;
static int *const g_009be608 = (int *)0x009BE608;
struct MSG {
    void *hwnd;
    unsigned int message;
    unsigned int wParam;
    long lParam;
    unsigned long time;
    long ptX;
    long ptY;
};

typedef int (__stdcall *PeekMessageFn)(MSG *, void *, unsigned int, unsigned int, unsigned int);
typedef int (__stdcall *TranslateMessageFn)(const MSG *);
typedef long (__stdcall *DispatchMessageFn)(const MSG *);

void __cdecl wait_task() {
    MSG msg1;
    MSG msg2;

    *g_009b7b9c = 0;
    do_video();
    do_net();
    check_net();

    PeekMessageFn peekMessage = reinterpret_cast<PeekMessageFn>(*g_00669358);
    TranslateMessageFn translateMessage = reinterpret_cast<TranslateMessageFn>(*g_0066935c);
    DispatchMessageFn dispatchMessage = reinterpret_cast<DispatchMessageFn>(*g_00669340);

    *g_009b7b9c = 1;
    for (;;) {
        do_video();
        check_net();
        do_net();
        if (!peekMessage(&msg1, 0, 0xf, 0xf, 1)) {
            break;
        }
        translateMessage(&msg1);
        dispatchMessage(&msg1);
        *g_009b7b9c = 1;
    }

    *g_009b7b9c = 0;
    if (!peekMessage(&msg2, 0, 0, 0x400, 1)) {
        if (!peekMessage(&msg2, 0, 0x400, 0xffff, 1)) {
            return;
        }
        if (msg2.message == 0x402) {
            Net *net = reinterpret_cast<Net *>(*g_009be608);
            if (net != 0) {
                net->handle_sys_msg();
            }
            return;
        }
    }
    translateMessage(&msg2);
    dispatchMessage(&msg2);
}
