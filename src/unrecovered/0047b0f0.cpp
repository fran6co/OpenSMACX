// ORIGINAL: 0x0047B0F0 BYTE_EXACT FILE
// LEVER: xor/or order - writing the dword-zero field first in the record
//   loop (matching `xor ebx,ebx` before `or ecx,-1`) reproduced the
//   register setup order; `#pragma function(memcpy)` kept the `call
//   _memcpy` instead of an inlined `rep movsd`.
// working copy - scaffold materialised by --work
// name      ?send_all_player_packets@@YAXH@Z
// size      222 bytes
// spans     0x0047B0F0-0x0047B1CE
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00630080 0x00645930

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_all_player_packets@@YAXH@Z  at 0x0047B0F0  (222 bytes)
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
static int *const g_0090dd14 = (int *)0x0090DD14;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d51c = (int *)0x0093D51C;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
#pragma function(memcpy)
void __cdecl send_all_player_packets(int a1) {
    char buf[0xa74];
    char *p = buf + 0x10;
    for (int i = 7; i != 0; --i) {
        *reinterpret_cast<int *>(p + 0x178) = 0;
        p[0] = static_cast<char>(0xff);
        p[1] = 0;
        p[2] = 0;
        p[3] = static_cast<char>(0xff);
        p[4] = 2;
        p += 0x17c;
    }
    if (a1 != -1 || *g_0093e8c0 != 0) {
        *reinterpret_cast<unsigned short *>(buf) = 0xf04;
        *reinterpret_cast<int *>(buf + 4) = -1;
        *reinterpret_cast<int *>(buf + 8) = 0;
        memcpy(buf + 0x10, g_0090dd14, 0xa64);
        if (a1 < 0) {
            *reinterpret_cast<unsigned short *>(buf) |= 0x4000;
            reinterpret_cast<Net *>(g_0093cd90)->send(buf, 0xa74, 0, 1);
            return;
        }
        *reinterpret_cast<unsigned short *>(buf) |= 0x1000;
        extern int g_93d51c[];
        int tableVal = *reinterpret_cast<int *>(
            reinterpret_cast<char *>(g_93d51c) + a1 * 0x19c);
        reinterpret_cast<Net *>(g_0093cd90)->send(buf, 0xa74, tableVal, 1);
    }
}
