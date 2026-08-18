// ORIGINAL: 0x00634200 sub_634200 0x00634200-0x0063427E FILE
// working copy - scaffold materialised by --work
// size      126 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645930
// indirect  0x00634208 0x00634215 0x00634271

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634200  at 0x00634200  (126 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void *memcpy(void *, const void *, unsigned int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;
struct Record {
    int val_a;
    int val_b;
    int val_c;
    int length;
    int data_ptr;
};

typedef void (__stdcall *CritFn)(void *);
extern "C" void *__cdecl queue_memcpy(void *, const void *, unsigned int);

class LockedQueue {
public:
    Record *record_;
    unsigned char pad_[8];
    unsigned char cs_[24];

    int method(int a1, int a2, int a3, int a4);
};

int LockedQueue::method(int a1, int a2, int a3, int a4) {
    void *cs = &cs_;
    reinterpret_cast<CritFn>(*g_0066917c)(cs);
    if (record_ == 0) {
        reinterpret_cast<CritFn>(*g_00669174)(cs);
        return 0;
    }
    if (a4 != 0) {
        *reinterpret_cast<int *>(a4) = record_->data_ptr;
    }
    int length = record_->length;
    if (length != 0 && a1 != 0) {
        queue_memcpy(reinterpret_cast<void *>(a1), reinterpret_cast<void *>(record_->data_ptr), length);
    }
    if (a2 != 0) {
        *reinterpret_cast<int *>(a2) = record_->val_b;
    }
    if (a3 != 0) {
        *reinterpret_cast<int *>(a3) = record_->val_a;
    }
    int result = record_->val_c;
    reinterpret_cast<CritFn>(*g_00669174)(cs);
    return result;
}
