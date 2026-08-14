// ORIGINAL: 0x0047B2E0 FILE
// RULED-OUT: manual dword copy loop for the 0x168-byte payload (MISMATCH #3,
//   push/mov prologue); a struct assignment from *(PayloadBlob*)g_0090e778
//   reaches #25 instead (je/jne polarity in the a2==0/else selection).
// working copy - scaffold materialised by --work
// name      ?send_session2_packet@@YAXHH@Z
// size      153 bytes
// spans     0x0047B2E0-0x0047B379
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00630080

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_session2_packet@@YAXHH@Z  at 0x0047B2E0  (153 bytes)
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
static int *const g_0090e778 = (int *)0x0090E778;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_0093d51c = (int *)0x0093D51C;
struct SessionRecord { unsigned int field0_; unsigned char pad_[0x198]; };
extern SessionRecord g_0093d51c_tbl[];

struct PayloadBlob { unsigned int words_[90]; };

struct Packet178 {
    unsigned short header_[8];
    PayloadBlob payload_;
};

void __cdecl send_session2_packet(int a1, int a2) {
    Packet178 pkt;
    pkt.payload_ = *(PayloadBlob *)g_0090e778;
    if (a2 < 0) {
        pkt.header_[0] = 0x4f03;
        ((Net *)g_0093cd90)->send(&pkt, 0x178, 0, 1);
        return;
    }
    unsigned int val;
    if (a2 == 0) {
        pkt.header_[0] = 0x2f03;
        val = *g_0093d4f4;
    } else {
        pkt.header_[0] = 0x1f03;
        val = g_0093d51c_tbl[a2].field0_;
    }
    ((Net *)g_0093cd90)->send(&pkt, 0x178, val, 1);

}
