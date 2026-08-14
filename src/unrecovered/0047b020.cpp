// ORIGINAL: 0x0047B020 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      ?send_faction_data@@YAXH@Z
// size      193 bytes
// spans     0x0047B020-0x0047B0E1
// prototype 
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00481FD0 0x005D4510 0x00630080 0x00645930

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_faction_data@@YAXH@Z  at 0x0047B020  (193 bytes)
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

struct MessageFactionData;
class Net;
class NetWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Net { public:
    int send(void *, int, unsigned long, int);
};

class NetWin { public:
    void setup_faction_data(MessageFactionData *);
};

extern "C" void *memcpy(void *, const void *, unsigned int);
void * mem_get(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0080a6f8 = (int *)0x0080A6F8;
static int *const g_00811e24 = (int *)0x00811E24;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_0093d51c = (int *)0x0093D51C;
#pragma function(memcpy)

void __cdecl send_faction_data(int a1) {
    if (*g_0093d4f0 == *g_0093d4f4) {
        if (*g_00811e24 == 0) {
            reinterpret_cast<NetWin *>(g_0080a6f8)->setup_faction_data(0);
        }
        int base = *g_00811e24;
        unsigned char count = *reinterpret_cast<unsigned char *>(base + 1);
        int inter = count * 133;
        int size = inter * 3 + 0x1a4;
        void *buf = mem_get(size);
        int copySize = inter * 3 + 0x194;
        *reinterpret_cast<short *>(buf) = 0xf0d;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 4) = -1;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(buf) + 8) = 0;
        memcpy(reinterpret_cast<char *>(buf) + 0x10, reinterpret_cast<void *>(*g_00811e24), copySize);
        if (a1 < 0) {
            reinterpret_cast<Net *>(g_0093cd90)->send(buf, size, 0, 1);
            return;
        }
        reinterpret_cast<Net *>(g_0093cd90)->send(
            buf, size,
            *reinterpret_cast<unsigned long *>(reinterpret_cast<char *>(g_0093d51c) + a1 * 0x19c),
            1);
    }
}
