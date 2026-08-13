// ORIGINAL: 0x0047B260 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      ?send_session_packet@@YAXHH@Z
// size      126 bytes
// spans     0x0047B260-0x0047B2DE
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00630080

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_session_packet@@YAXHH@Z  at 0x0047B260  (126 bytes)
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


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_0093d51c = (int *)0x0093D51C;
struct SessionPacketBuf {
    unsigned short header;
    unsigned short pad[7];
    int payload[6];
};

void __cdecl send_session_packet(int a1, int a2) {
    SessionPacketBuf buf;
    for (int i = 0; i < 6; i = i + 1) {
        buf.payload[i] = g_0090e8e0[i];
    }

    if (a2 < 0) {
        buf.header = 0x4f02;
        ((Net *)g_0093cd90)->send(&buf, 0x28, 0, 1);
        return;
    }

    unsigned long code;
    if (a2 != 0) {
        code = *(unsigned long *)((char *)g_0093d51c + a2 * 0x19c);
        buf.header = 0x1f02;
    } else {
        code = *(unsigned long *)g_0093d4f4;
        buf.header = 0x2f02;
    }
    ((Net *)g_0093cd90)->send(&buf, 0x28, code, 1);
}
