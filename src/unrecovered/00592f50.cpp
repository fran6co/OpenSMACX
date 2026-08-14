// ORIGINAL: 0x00592F50 FILE
// RULED-OUT: 10-arg __cdecl packing a 0x90-byte NetDaemon message on the
//            stack; MISMATCH #29, byte buffer vs epilogue push/lea/push
//            around the send_message call - not iterated further.
// working copy - scaffold materialised by --work
// name      sub_592f50
// size      153 bytes
// spans     0x00592F50-0x00592FE9
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532940
// indirect  0x00592F70

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_592f50  at 0x00592F50  (153 bytes)
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
extern "C" void __cdecl sub_592f50(unsigned short a1, int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7, unsigned int a8, unsigned int a9, unsigned int a10) {
    typedef unsigned int (__stdcall *TimeGetTimeFn)();
    unsigned char buf[0x90];
    *reinterpret_cast<unsigned short *>(buf + 0x00) = a1;
    *reinterpret_cast<unsigned int *>(buf + 0x04) = *g_00939284;
    *reinterpret_cast<unsigned int *>(buf + 0x08) = (*reinterpret_cast<TimeGetTimeFn *>(g_00669368))();
    *reinterpret_cast<unsigned int *>(buf + 0x0c) = *g_0093e8bc;
    *reinterpret_cast<unsigned int *>(buf + 0x10) = a3;
    *reinterpret_cast<unsigned int *>(buf + 0x14) = a4;
    *reinterpret_cast<unsigned int *>(buf + 0x18) = a5;
    *reinterpret_cast<unsigned int *>(buf + 0x1c) = a6;
    *reinterpret_cast<unsigned int *>(buf + 0x20) = a7;
    *reinterpret_cast<unsigned int *>(buf + 0x24) = a8;
    *reinterpret_cast<unsigned int *>(buf + 0x28) = a9;
    *reinterpret_cast<unsigned int *>(buf + 0x2c) = a10;
    ++*g_0093f664;
    reinterpret_cast<NetDaemon *>(g_0093cd90)->send_message(reinterpret_cast<char *>(buf), 0x90, a2);
    --*g_0093f664;
}
