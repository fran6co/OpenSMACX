// ORIGINAL: 0x00633F70 ?get@NetFifo@@QAEHPAXPAIPAHPAI@Z 0x00633F70-0x0063401D FILE
// TRIED: local FifoNode struct over the opaque node pointer, CS via `self+0xc` and a stdcall function-pointer cast for Enter/LeaveCS. MISMATCH #1 push/mov, prologue register-save shape only.
// working copy - scaffold materialised by --work
// size      173 bytes
// prototype int (__thiscall ?get@NetFifo@@QAEHPAXPAIPAHPAI@Z)(NetFifo* this, void*, unsigned int*, int*, unsigned int*)
// callers   3   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2 0x00645930
// indirect  0x00633F79 0x00633F86 0x0063400E

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get@NetFifo@@QAEHPAXPAIPAHPAI@Z  at 0x00633F70  (173 bytes)
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
extern "C" void *memcpy(void *, const void *, unsigned int);
extern "C" void free(void *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;

class NetFifo { public:
    int m_00633f70(void *, unsigned int *, int *, unsigned int *);
};
struct FifoNode {
    int field0_;
    int field4_;
    int field8_;
    void *bufC_;
    unsigned int len10_;
    FifoNode *next14_;
};

int NetFifo::m_00633f70(void * a1, unsigned int * a2, int * a3, unsigned int * a4) {
    typedef void (__stdcall *CsFn)(void *);
    char *self = reinterpret_cast<char *>(this);
    void *cs = self + 0xc;
    (*(CsFn *)g_0066917c)(cs);
    FifoNode *node = *reinterpret_cast<FifoNode **>(self);
    if (node == 0) {
        (*(CsFn *)g_00669174)(cs);
        return 0;
    }
    if (a4 != 0) {
        *a4 = node->len10_;
    }
    if ((node->bufC_ != 0) && (a1 != 0)) {
        memcpy(a1, node->bufC_, node->len10_);
        if (node->bufC_ != 0) {
            free(node->bufC_);
        }
        node->bufC_ = 0;
    }
    *reinterpret_cast<FifoNode **>(self) = node->next14_;
    if (a2 != 0) {
        *a2 = (unsigned int)node->field4_;
    }
    if (a3 != 0) {
        *a3 = node->field0_;
    }
    int ret = node->field8_;
    if (node != 0) {
        free(node);
    }
    *reinterpret_cast<int *>(self + 8) -= 1;
    (*(CsFn *)g_00669174)(cs);
    return ret;
}
