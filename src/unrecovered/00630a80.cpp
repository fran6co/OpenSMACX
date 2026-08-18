// ORIGINAL: 0x00630A80 sub_630a80 0x00630A80-0x00630AFC FILE
// RULED-OUT: `__stdcall` free-function contract (ecx read at offset 0x6fc/ 0x10c with no stack access -> real receiver, made a `TimeoutRecv` thiscall member). Original spills its cached `timeGetTime()` start value into the (now-unused) `a1` stack argument slot instead of a fresh local, which this source form does not reproduce (extra `sub esp,N` for a real local).
// working copy - scaffold materialised by --work
// size      124 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00633F70
// indirect  0x00630A9D 0x00630AA5 0x00630AD1

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_630a80  at 0x00630A80  (124 bytes)
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

class NetFifo;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetFifo { public:
    int m_00633f70(void *, unsigned int *, int *, unsigned int *);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_009bc4bc = (int *)0x009BC4BC;
static int *const g_009be600 = (int *)0x009BE600;
class TimeoutRecv { public:
    int get_with_timeout(int a1, int a2);
};

typedef unsigned int (__stdcall *TimeGetTimeFn)();

int TimeoutRecv::get_with_timeout(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (*g_009be600 == 0 || a1 == 0) {
        return 0;
    }
    TimeGetTimeFn time_get_time = (TimeGetTimeFn)*g_00669368;
    unsigned int start = time_get_time();
    unsigned int now = time_get_time();
    if (now - start >= *reinterpret_cast<unsigned int *>(self + 0x6fc)) {
        return 0;
    }
    NetFifo *fifo = reinterpret_cast<NetFifo *>(self + 0x10c);
    do {
        int result = fifo->m_00633f70(reinterpret_cast<void *>(g_009bc4bc),
                                       reinterpret_cast<unsigned int *>(a1),
                                       0,
                                       reinterpret_cast<unsigned int *>(a2));
        if (result != 0) {
            return reinterpret_cast<int>(g_009bc4bc);
        }
        now = time_get_time();
    } while (now - start < *reinterpret_cast<unsigned int *>(self + 0x6fc));
    return 0;
}
