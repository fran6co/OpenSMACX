// ORIGINAL: 0x005389F0 ??0NetDaemon@@QAE@XZ 0x005389F0-0x00538B58;0x0065DF50-0x0065DF62 FILE
// size      378 bytes
// prototype void (__thiscall ??0NetDaemon@@QAE@XZ)(NetDaemon* this)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E2490 0x004E25B0 0x0058FF70 0x005900D0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005389F0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005389f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0NetDaemon@@QAE@XZ  at 0x005389F0  (378 bytes)
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
class Lock;
class NetDaemon;
class PlayerLock;

// ---- callees, declared and never defined (a definition would be inlined) ----
class AlphaNet { public:
    uint32_t alignment_;
    uint8_t data_[0x149C];
    AlphaNet();
    void close();
};
class Lock { public:
    void clear();
};
class PlayerLock { public:
    void clear();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0065df58 = (int *)0x0065DF58;
static int *const g_0066ef0c = (int *)0x0066EF0C;
static int *const g_0067c248 = (int *)0x0067C248;

class NetDaemon { public:
    NetDaemon();
};

inline void *operator new(unsigned int, void *place) { return place; }

NetDaemon::NetDaemon() {
    char *const self = reinterpret_cast<char *>(this);

    new (self) AlphaNet();

    char *p = self + 0x14a0;
    int32_t n = 8;
    do {
        reinterpret_cast<PlayerLock *>(p)->clear();
        p += 0x1c;
    } while (--n);

    reinterpret_cast<Lock *>(self + 0x14a0)->clear();

    char *q = self + 0x158c;
    int32_t m = 0x18;
    do {
        *reinterpret_cast<uint8_t *>(q) = 0xff;
        *reinterpret_cast<int16_t *>(q + 8) = 0;
        *reinterpret_cast<int32_t *>(q + 0x5a0) = 0;
        q += 0x3c;
    } while (--m);

    *reinterpret_cast<int32_t *>(self + 0x1b30) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b34) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b38) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b3c) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b60) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b5c) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b6c) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b68) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b7c) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1ba8) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1ba0) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1ba4) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1bcc) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1bc8) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b50) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b54) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b58) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1bc4) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1bc0) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1bb0) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1bac) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1bb4) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b40) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b44) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1bd8) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1bd4) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b48) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b4c) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1b78) = 0;

    *reinterpret_cast<void **>(self) = g_0066ef0c;
    *reinterpret_cast<int32_t *>(self + 0x1bd0) = 0xff;
    for (int32_t i = 0; i < 8; i++) {
        reinterpret_cast<int32_t *>(self + 0x1b80)[i] = 0;
    }

    reinterpret_cast<Lock *>(self + 0x14a0)->clear();
    reinterpret_cast<AlphaNet *>(self)->close();
}
