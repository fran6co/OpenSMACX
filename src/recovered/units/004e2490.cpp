// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004E2490
// name           ??0AlphaNet@@QAE@XZ
// size           309 bytes
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e2490/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0AlphaNet@@QAE@XZ  at 0x004E2490  (309 bytes)
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

class AlphaNet;
class Heap;
typedef void * LPVOID;
class Net;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    Heap();
};
class Net { public:
    Net();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0065c626 = (int *)0x0065C626;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_00669404 = (int *)0x00669404;
static int *const g_00669408 = (int *)0x00669408;
static int *const g_0066eacc = (int *)0x0066EACC;
static int *const g_0066eae4 = (int *)0x0066EAE4;
static int *const g_0066eae8 = (int *)0x0066EAE8;
static int *const g_0066eaf0 = (int *)0x0066EAF0;
static int *const g_0067b0b0 = (int *)0x0067B0B0;
static int *const g_009b3374 = (int *)0x009B3374;

class AlphaNet { public:
    uint32_t alignment_;
    uint8_t data_[0x149C];

    AlphaNet();
};

inline void *operator new(unsigned int, void *p) { return p; }

AlphaNet::AlphaNet() {
    char *self = reinterpret_cast<char *>(this);

    new (self) Net();

    unsigned char *p = reinterpret_cast<unsigned char *>(self + 0x792);
    for (int i = 8; i != 0; i--) {
        p[-2] = 0xff;
        p[-1] = 0;
        p[0] = 0;
        p[1] = 0xff;
        p[2] = 2;
        p += 0x19c;
    }

    *reinterpret_cast<int *>(self + 0x1450) = reinterpret_cast<int>(g_0066eaf0);
    *reinterpret_cast<int *>(self + 0x1484) = reinterpret_cast<int>(g_006693ac);
    *reinterpret_cast<int *>(self + 0x1488) = *g_009b3374;
    *g_009b3374 = 0;

    *reinterpret_cast<int *>(self + 0x144c) = reinterpret_cast<int>(g_00669408);
    int *listHead = *reinterpret_cast<int **>(self + 0x1450);
    int listOff = *reinterpret_cast<int *>(reinterpret_cast<char *>(listHead) + 4);
    *reinterpret_cast<int *>(self + 0x1450 + listOff) = reinterpret_cast<int>(g_00669404);

    *reinterpret_cast<int *>(self + 0x1454) = 0;
    *reinterpret_cast<int *>(self + 0x1458) = 0;
    *reinterpret_cast<int *>(self + 0x145c) = 0;
    *reinterpret_cast<int *>(self + 0x1460) = 0;
    *reinterpret_cast<int *>(self + 0x1464) = 0;

    *reinterpret_cast<int *>(self + 0x144c) = reinterpret_cast<int>(g_0066eae8);
    int *listHead2 = *reinterpret_cast<int **>(self + 0x1450);
    int listOff2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(listHead2) + 4);
    *reinterpret_cast<int *>(self + 0x1450 + listOff2) = reinterpret_cast<int>(g_0066eae4);

    new (self + 0x148c) Heap();

    *reinterpret_cast<int *>(self) = reinterpret_cast<int>(g_0066eacc);

    unsigned char *q = reinterpret_cast<unsigned char *>(self + 0x78c);
    for (int j = 8; j != 0; j--) {
        *reinterpret_cast<int *>(q) = 0;
        q += 0x19c;
    }
    *reinterpret_cast<int *>(self + 0x768) = 0;
}
