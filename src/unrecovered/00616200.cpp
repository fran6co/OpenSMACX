// ORIGINAL: 0x00616200 BYTE_EXACT FILE
// LEVER: two indirect calls through fixed function-pointer slots, gated on
//        count_ < 0x32; store order matches declaration except resolution_
//        gets the immediate 5, not 0
// working copy - scaffold materialised by --work
// name      ??1Time@@QAE@XZ
// size      96 bytes
// spans     0x00616200-0x00616260
// prototype void (__thiscall ??1Time@@QAE@XZ)(Time* this)
// callers   30   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD370
// indirect  0x0061621E 0x0061622C
// notes     Staged hybrid export redirect calls the source-owned destructor

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1Time@@QAE@XZ  at 0x00616200  (96 bytes)
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

class Time;
typedef unsigned int UINT_PTR;

// ---- callees, declared and never defined (a definition would be inlined) ----
void flush_timer();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006692f0 = (int *)0x006692F0;
static int *const g_00669380 = (int *)0x00669380;
static int *const g_009b7b28 = (int *)0x009B7B28;

class Time { public:
    int unk_tgl_;
    UINT_PTR id_event_;
    void * callback1_;
    void * callback2_;
    int cb_param2_;
    int cb_param1_;
    uint32_t count_;
    int unk_1_;
    uint32_t resolution_;
    int unk_2_;

    ~Time();
};
typedef void (__stdcall *PFN_00669380)(UINT_PTR);
typedef void (__stdcall *PFN_006692f0)(int, UINT_PTR);

Time::~Time() {
    int had_flush = ~unk_tgl_ & 1;
    if (id_event_ != 0) {
        if (count_ < 0x32) {
            ((PFN_00669380)*g_00669380)(id_event_);
        } else {
            ((PFN_006692f0)*g_006692f0)(*g_009b7b28, id_event_);
        }
        id_event_ = 0;
    }
    if (had_flush != 0) {
        flush_timer();
    }
    callback1_ = 0;
    unk_tgl_ = 0;
    callback2_ = 0;
    cb_param2_ = 0;
    cb_param1_ = 0;
    count_ = 0;
    resolution_ = 5;
    unk_1_ = 0;
    unk_2_ = 0;
}
