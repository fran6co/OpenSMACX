// ORIGINAL: 0x00630B00 sub_630b00 0x00630B00-0x00630B7C FILE
// TRIED: kept the contract's `__stdcall(int,int)` head - the disasm sets `ebx = ecx` on entry and reads `[ebx+0x6fc]`/`[ebx+0x10c]` with no stack access for a receiver, so it is really a __thiscall member; rewritten as one. MISMATCH is at #0 (bigger frame, 155 vs 124 bytes), not tried further.
// working copy - scaffold materialised by --work
// size      124 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00633D90
// indirect  0x00630B1D 0x00630B25 0x00630B51

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_630b00  at 0x00630B00  (124 bytes)
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
    int get(void *, unsigned int *, int *, unsigned int *);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_009bc4bc = (int *)0x009BC4BC;
static int *const g_009be600 = (int *)0x009BE600;
typedef unsigned long (__stdcall *TimeGetTimeFn)();

class NetWaiter { public:
    int wait(unsigned int *, unsigned int *);
};

int NetWaiter::wait(unsigned int *a1, unsigned int *a2) {
    char *self = reinterpret_cast<char *>(this);
    if (*g_009be600 != 0 && a1 != 0) {
        TimeGetTimeFn timeGetTime_ = (TimeGetTimeFn)*g_00669368;
        unsigned long start = timeGetTime_();
        unsigned long now = timeGetTime_();
        if (now - start < *reinterpret_cast<unsigned int *>(self + 0x6fc)) {
            NetFifo *fifo = reinterpret_cast<NetFifo *>(self + 0x10c);
            do {
                int rc = fifo->get(reinterpret_cast<void *>(g_009bc4bc), a1, 0, a2);
                if (rc != 0) {
                    return (int)g_009bc4bc;
                }
                now = timeGetTime_();
            } while (now - start < *reinterpret_cast<unsigned int *>(self + 0x6fc));
        }
    }
    return 0;
}
