// ORIGINAL: 0x0047B1D0 FILE
// working copy - scaffold materialised by --work
// name      sub_47b1d0
// size      140 bytes
// spans     0x0047B1D0-0x0047B25C
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00630080 0x00645930

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_47b1d0  at 0x0047B1D0  (140 bytes)
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
    int send(void *, int, unsigned long, int);
};

extern "C" void *memcpy(void *, const void *, unsigned int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
void __cdecl sub_47b1d0(int a1, void * a2) {
    struct {
        short header;
        short pad;
        int p1;
        int zero;
        int gap;
        unsigned char addr[5];
        unsigned char tail[0x177];
    } msg;

    msg.addr[0] = 0xff;
    msg.addr[3] = 0xff;
    msg.addr[1] = 0;
    msg.addr[2] = 0;
    msg.addr[4] = 2;

    if (a1 > 0 && a2 != 0) {
        msg.zero = 0;
        msg.header = 0x2f04;
        msg.p1 = a1;
        memcpy(msg.addr, a2, 0x17c);
        reinterpret_cast<Net *>(g_0093cd90)->send(&msg, 0x18c, *g_0093d4f4, 1);
    }
}
