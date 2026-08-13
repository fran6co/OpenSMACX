// ORIGINAL: 0x00593110 FILE
// working copy - scaffold materialised by --work
// name      ?message_landmark@@YAXHHPADHHHH@Z
// size      128 bytes
// spans     0x00593110-0x00593190
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532940 0x00645460
// indirect  0x00593127

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?message_landmark@@YAXHHPADHHHH@Z  at 0x00593110  (128 bytes)
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

extern "C" char *strcpy(char *, const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8bc = (int *)0x0093E8BC;
static int *const g_0093f664 = (int *)0x0093F664;
void __cdecl message_landmark(int a1, int a2, char * a3, int a4, int a5, int a6, int a7) {
    typedef unsigned long (__stdcall *TimeGetTimeFn)();
    char buf[0x40];
    *reinterpret_cast<short *>(buf) = static_cast<short>(a1);
    *reinterpret_cast<int *>(buf + 4) = *g_00939284;
    *reinterpret_cast<unsigned long *>(buf + 8) = ((TimeGetTimeFn)(*g_00669368))();
    *reinterpret_cast<int *>(buf + 0x10) = a4;
    *reinterpret_cast<int *>(buf + 0x1c) = a7;
    *reinterpret_cast<int *>(buf + 0xc) = *g_0093e8bc;
    *reinterpret_cast<int *>(buf + 0x14) = a5;
    *reinterpret_cast<int *>(buf + 0x18) = a6;
    if (a3) {
        strcpy(buf + 0x20, a3);
    }
    ++*g_0093f664;
    reinterpret_cast<NetDaemon *>(g_0093cd90)->send_message(buf, 0x40, a2);
    --*g_0093f664;
}
