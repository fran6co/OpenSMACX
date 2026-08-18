// ORIGINAL: 0x00593090 ?message_lock@@YAXHHHHHHHH@Z 0x00593090-0x00593108 FILE
// working copy - scaffold materialised by --work
// size      120 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532940
// indirect  0x005930A7

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?message_lock@@YAXHHHHHHHH@Z  at 0x00593090  (120 bytes)
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
    void send_message(char *, unsigned long, int);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8bc = (int *)0x0093E8BC;
static int *const g_0093f664 = (int *)0x0093F664;
typedef unsigned long (__stdcall *TimeGetTimeFn)();

extern int g_lock_count;

void __cdecl message_lock(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    struct {
        short a1_;
        int field4_;
        unsigned long time_;
        int fieldC_;
        int a3_;
        int a4_;
        int a2_;
        int a6_;
        int a7_;
        int a5_;
    } msg;

    msg.a1_ = static_cast<short>(a1);
    msg.field4_ = *g_00939284;
    msg.time_ = reinterpret_cast<TimeGetTimeFn>(*g_00669368)();
    msg.fieldC_ = *g_0093e8bc;
    msg.a3_ = a3;
    msg.a4_ = a4;
    msg.a2_ = a2;
    msg.a6_ = a6;
    msg.a7_ = a7;
    msg.a5_ = a5;

    ++g_lock_count;
    reinterpret_cast<NetDaemon *>(g_0093cd90)->send_message(
        reinterpret_cast<char *>(&msg), sizeof(msg), a8);
    --g_lock_count;
}
