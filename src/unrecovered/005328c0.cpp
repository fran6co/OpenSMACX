// ORIGINAL: 0x005328C0 FILE
// working copy - scaffold materialised by --work
// name      ?end_of_event@NetDaemon@@QAEHHH@Z
// size      116 bytes
// spans     0x005328C0-0x00532934
// prototype int (__thiscall ?end_of_event@NetDaemon@@QAEHHH@Z)(NetDaemon* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00592EE0 0x006262F0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?end_of_event@NetDaemon@@QAEHHH@Z  at 0x005328C0  (116 bytes)
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
void log_say(char *, int, int, int);
void message_data(int, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068c570 = (int *)0x0068C570;
static int *const g_00939284 = (int *)0x00939284;

class NetDaemon { public:
    int end_of_event(int, int);
};
int NetDaemon::end_of_event(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x1b30) == 0) {
        *reinterpret_cast<int *>(self + 0x1b2c) = a2;
    }
    if (a1 == *g_00939284) {
        if (*reinterpret_cast<int *>(self + 0x1bc0) != 0) {
            log_say(reinterpret_cast<char *>(g_0068c570), 0, 0, 0);
            *reinterpret_cast<int *>(self + 0x1bc0) = 0;
            message_data(0x2214, 0, 0, 0, 0, 0);
            return 0;
        }
        *reinterpret_cast<int *>(self + 0x1bb0) = 0;
    }
    return 0;
}
