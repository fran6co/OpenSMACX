// ORIGINAL: 0x0051F0C0 BYTE_EXACT FILE
// LEVER: `while (daemon->receive() != 0) {}` reproduces the two-call-site
//        loop shape (one test before the loop, one inside) purely from the
//        standard while-loop rotation - no need to hand-duplicate the call.
// working copy - scaffold materialised by --work
// name      ?help_spock@@YAXXZ
// size      234 bytes
// spans     0x0051F0C0-0x0051F1AA
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00530320 0x00532E00

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?help_spock@@YAXXZ  at 0x0051F0C0  (234 bytes)
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
    int receive();
    void synch(short, int, int, int, char *, int, short);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_0093f670 = (int *)0x0093F670;
static int *const g_0093f674 = (int *)0x0093F674;
void __cdecl help_spock() {
    if (*g_0093e8c0 != 0) {
        NetDaemon *daemon = reinterpret_cast<NetDaemon *>(g_0093cd90);

        daemon->synch(0x25, 0, 0, 0, 0, 1, 0x2101);
        while (daemon->receive() != 0) {
        }

        daemon->synch(0x12, 0, 0, 0, 0, 1, 0x2101);
        while (daemon->receive() != 0) {
        }

        daemon->synch(0x26, 0, 0, 0, 0, 1, 0x2101);
        while (daemon->receive() != 0) {
        }

        daemon->synch(0x14, 0, 0, 0, 0, 1, 0x2101);
        while (daemon->receive() != 0) {
        }
    }

    *g_0093f670 = 0;
    *g_0093f674 = 0;
}
