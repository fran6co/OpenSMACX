// ORIGINAL: 0x0062D6A0 FILE
// name      ??0Net@@QAE@XZ
// size      635 bytes
// spans     0x0062D6A0-0x0062D8DC;0x006634A0-0x006634DF
// prototype void (__thiscall ??0Net@@QAE@XZ)(Net* this)
// callers   1   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C8CC0 0x006339C0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0062D6A0
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0062d6a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0Net@@QAE@XZ  at 0x0062D6A0  (635 bytes)
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
class NetFifo;
class VoiceTx;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetFifo { public:
    NetFifo();
};
class VoiceTx { public:
    VoiceTx();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006634d5 = (int *)0x006634D5;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006693f0 = (int *)0x006693F0;
static int *const g_006693f4 = (int *)0x006693F4;
static int *const g_00669418 = (int *)0x00669418;
static int *const g_0066941c = (int *)0x0066941C;
static int *const g_00669424 = (int *)0x00669424;
static int *const g_0066942c = (int *)0x0066942C;
static int *const g_00669430 = (int *)0x00669430;
static int *const g_0066eaf8 = (int *)0x0066EAF8;
static int *const g_0066eafc = (int *)0x0066EAFC;
static int *const g_0066eb04 = (int *)0x0066EB04;
static int *const g_00670dcc = (int *)0x00670DCC;
static int *const g_006800e8 = (int *)0x006800E8;
static int *const g_009b3374 = (int *)0x009B3374;
static int *const g_009be600 = (int *)0x009BE600;
static int *const g_009be608 = (int *)0x009BE608;

class Net { public:
    Net();
};

inline void *operator new(unsigned int, void *p) { return p; }

Net::Net() {
    char *self = reinterpret_cast<char *>(this);

    {
        new (self + 0x58) VoiceTx();

        *reinterpret_cast<void **>(self + 0xb4) = g_00670dcc;
        *reinterpret_cast<void **>(self + 0xcc) = g_006693ac;
        *reinterpret_cast<int *>(self + 0xd0) = *g_009b3374;
        *g_009b3374 = 0;

        *reinterpret_cast<void **>(self + 0xb0) = g_0066eafc;
        {
            int vtbl1 = *reinterpret_cast<int *>(self + 0xb4);
            int adj1 = *reinterpret_cast<int *>(vtbl1 + 4);
            *reinterpret_cast<void **>(self + 0xb4 + adj1) = g_0066eaf8;
        }
        *reinterpret_cast<int *>(self + 0xb8) = 0;
        *reinterpret_cast<int *>(self + 0xbc) = 0;
        *reinterpret_cast<int *>(self + 0xc0) = 0;
        *reinterpret_cast<int *>(self + 0xc4) = 0;
        *reinterpret_cast<int *>(self + 0xc8) = 0;

        new (self + 0xe8) NetFifo();
        new (self + 0x10c) NetFifo();
        new (self + 0x130) NetFifo();
    }

    *reinterpret_cast<void **>(self + 0x700) = g_00669430;
    *reinterpret_cast<void **>(self + 0x758) = g_006693ac;
    *reinterpret_cast<int *>(self + 0x75c) = *g_009b3374;
    *g_009b3374 = 0;

    *reinterpret_cast<void **>(self + 0x730) = g_00669424;
    *reinterpret_cast<void **>(self + 0x750) = g_006693ac;
    *reinterpret_cast<int *>(self + 0x754) = *g_009b3374;
    *g_009b3374 = 0;

    {
        int vtbl2 = *reinterpret_cast<int *>(self + 0x730);
        *reinterpret_cast<void **>(self + 0x72c) = g_006693f4;
        int adj2 = *reinterpret_cast<int *>(vtbl2 + 4);
        *reinterpret_cast<void **>(self + 0x730 + adj2) = g_006693f0;
    }

    {
        int vtbl3 = *reinterpret_cast<int *>(self + 0x730);
        *reinterpret_cast<int *>(self + 0x734) = 0;
        *reinterpret_cast<int *>(self + 0x738) = 0;
        *reinterpret_cast<int *>(self + 0x73c) = 0;
        *reinterpret_cast<int *>(self + 0x740) = 0;
        *reinterpret_cast<int *>(self + 0x744) = 0;
        *reinterpret_cast<void **>(self + 0x72c) = g_0066941c;
        int adj3 = *reinterpret_cast<int *>(vtbl3 + 4);
        *reinterpret_cast<void **>(self + 0x730 + adj3) = g_00669418;
    }

    {
        int vtbl4 = *reinterpret_cast<int *>(self + 0x700);
        int adj4 = *reinterpret_cast<int *>(vtbl4 + 4);
        *reinterpret_cast<void **>(self + 0x700 + adj4) = g_0066942c;
    }

    *reinterpret_cast<void **>(self) = g_0066eb04;
    *g_009be608 = reinterpret_cast<int>(self);
    *reinterpret_cast<int *>(self + 0xdc) = 0;
    *reinterpret_cast<int *>(self + 0xe0) = 0x4e20;
    *g_009be600 = 0;
    *reinterpret_cast<int *>(self + 0x6dc) = 0;
    *reinterpret_cast<int *>(self + 0x760) = 0;
    *reinterpret_cast<int *>(self + 0x764) = 0;
    *reinterpret_cast<int *>(self + 0x6e0) = 0;
    *reinterpret_cast<int *>(self + 0x6e4) = 0;
    *reinterpret_cast<int *>(self + 0x6fc) = 0xc8;

    char *slot = self + 0x158;
    for (int n = 0x10; n != 0; --n) {
        *reinterpret_cast<int *>(slot - 4) = 0;
        *reinterpret_cast<int *>(slot) = 0;
        *reinterpret_cast<int *>(slot + 4) = 0;
        *reinterpret_cast<unsigned char *>(slot + 0x10) = 0;
        *reinterpret_cast<int *>(slot + 0xc) = 0;
        slot += 0x58;
    }

    *reinterpret_cast<int *>(self + 0x6d4) = 0x64;
    *reinterpret_cast<int *>(self + 0xd8) = 0;
    *reinterpret_cast<int *>(self + 0xe4) = 1;
    *reinterpret_cast<int *>(self + 0x48) = 0;
    *reinterpret_cast<int *>(self + 0x4c) = 0;
    *reinterpret_cast<int *>(self + 0x50) = 0;
    *reinterpret_cast<int *>(self + 0x54) = 0;
    *reinterpret_cast<int *>(self + 0x20) = 0;
    *reinterpret_cast<int *>(self + 0x24) = 0;
    *reinterpret_cast<int *>(self + 0x1c) = 0;
    *reinterpret_cast<int *>(self + 4) = 0;
    *reinterpret_cast<int *>(self + 8) = 0;
    *reinterpret_cast<int *>(self + 0xc) = 0;
    *reinterpret_cast<int *>(self + 0x10) = 0;
    *reinterpret_cast<int *>(self + 0x14) = 0;
    *reinterpret_cast<int *>(self + 0x18) = 0;
    *reinterpret_cast<int *>(self + 0x28) = 0;
}
