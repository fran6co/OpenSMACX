// PRESERVED UNIT - measured SHARED_TAIL.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00538B60
// name           ??1AlphaNet@@QAE@XZ
// size           606 bytes
// measured tier  SHARED_TAIL
// refusal        1 span(s) are COMDAT-folded and claimed by another function; no per-function verdict is well defined
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00538b60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1AlphaNet@@QAE@XZ  at 0x00538B60  (606 bytes)
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
class JackalVoiceRx;
typedef void * LPVOID;
class Net;
class NetFifo;
class VoiceTx;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    void shutdown();
};
class JackalVoiceRx { public:
    ~JackalVoiceRx();
};
class Net { public:
    void close();
};
class NetFifo { public:
    ~NetFifo();
};
class VoiceTx { public:
    ~VoiceTx();
};
extern "C" int __cdecl fn_00402970();
extern "C" int __cdecl sub_401be0();
extern "C" int __cdecl sub_402dd0();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0065dffb = (int *)0x0065DFFB;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_00669404 = (int *)0x00669404;
static int *const g_00669408 = (int *)0x00669408;
static int *const g_00669418 = (int *)0x00669418;
static int *const g_0066941c = (int *)0x0066941C;
static int *const g_0066eacc = (int *)0x0066EACC;
static int *const g_0066eae4 = (int *)0x0066EAE4;
static int *const g_0066eae8 = (int *)0x0066EAE8;
static int *const g_0066eaf8 = (int *)0x0066EAF8;
static int *const g_0066eafc = (int *)0x0066EAFC;
static int *const g_0066eb04 = (int *)0x0066EB04;
static int *const g_0067c270 = (int *)0x0067C270;
static int *const g_009b3374 = (int *)0x009B3374;

class AlphaNet { public:
    uint32_t alignment_;
    uint8_t data_[0x149C];

    void close();
    ~AlphaNet();
};

// The two `call fn_00402970` / `call sub_401be0` sites pass an object in
// ECX (a __thiscall member on an opaque helper type), but the scaffold's
// declaration is a nullary __cdecl free function - the emitter's plain-call
// scan doesn't distinguish the two. Reached instead through tiny shims of
// the right calling convention.
class StringStructX { public:
    int remove_all();
};
class Sub401be0X { public:
    int method();
};

AlphaNet::~AlphaNet() {
    char *self = reinterpret_cast<char *>(this);

    close();

    reinterpret_cast<Heap *>(self + 0x148c)->shutdown();

    reinterpret_cast<StringStructX *>(self + 0x144c)->remove_all();
    *reinterpret_cast<uint32_t *>(self + 0x1460) = 0;
    reinterpret_cast<StringStructX *>(self + 0x144c)->remove_all();
    *reinterpret_cast<uint32_t *>(self + 0x1460) = 0;

    {
        int v = *reinterpret_cast<int *>(self + 0x1488);
        *reinterpret_cast<void **>(self + 0x1484) = reinterpret_cast<void *>(g_006693ac);
        *g_009b3374 = v;
    }

    reinterpret_cast<Net *>(self)->close();

    reinterpret_cast<StringStructX *>(self + 0x72c)->remove_all();
    *reinterpret_cast<uint32_t *>(self + 0x740) = 0;

    reinterpret_cast<Sub401be0X *>(self + 0x748)->method();

    {
        int v = *reinterpret_cast<int *>(self + 0x754);
        *reinterpret_cast<void **>(self + 0x750) = reinterpret_cast<void *>(g_006693ac);
        *g_009b3374 = v;
    }

    {
        int v = *reinterpret_cast<int *>(self + 0x75c);
        *reinterpret_cast<void **>(self + 0x758) = reinterpret_cast<void *>(g_006693ac);
        *g_009b3374 = v;
    }

    reinterpret_cast<NetFifo *>(self + 0x130)->~NetFifo();
    reinterpret_cast<NetFifo *>(self + 0x10c)->~NetFifo();
    reinterpret_cast<NetFifo *>(self + 0xe8)->~NetFifo();

    {
        char *node = self + 0xb0;
        *reinterpret_cast<void **>(node) = reinterpret_cast<void *>(g_0066eafc);
        int adj = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0xb4) + 4);
        *reinterpret_cast<void **>(self + 0xb4 + adj) = reinterpret_cast<void *>(g_0066eaf8);
        reinterpret_cast<JackalVoiceRx *>(node)->~JackalVoiceRx();
        *reinterpret_cast<uint32_t *>(node + 0x14) = 0;
    }

    {
        int v = *reinterpret_cast<int *>(self + 0xd0);
        *reinterpret_cast<void **>(self + 0xcc) = reinterpret_cast<void *>(g_006693ac);
        *g_009b3374 = v;
    }

    reinterpret_cast<VoiceTx *>(self + 0x58)->~VoiceTx();
}
