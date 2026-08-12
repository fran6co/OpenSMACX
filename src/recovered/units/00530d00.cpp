// ORIGINAL: 0x00530D00 BYTE_EXACT FILE
// name      ?unlock@NetDaemon@@QAEXXZ
// size      78 bytes
// spans     0x00530D00-0x00530D4E
// prototype void (__thiscall ?unlock@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   10   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0 0x006262F0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00530D00
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00530d00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?unlock@NetDaemon@@QAEXXZ  at 0x00530D00  (78 bytes)
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
void __cdecl log_say(char *, int, int, int);
void __cdecl message_data(int, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
// Passed to log_say only as an address - extern keeps the literal from
// folding away under /O2.
extern char g_0068c2dc[];
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093f660 = (int *)0x0093F660;

class NetDaemon { public:
    void unlock();
};

void NetDaemon::unlock() {
    char *self = reinterpret_cast<char *>(this);
    if (*g_0093f660 != 0) {
        log_say(g_0068c2dc, *g_00939284, 0, 0);
        message_data(0x2212, 0, 0, 0, 0, 0);
        *reinterpret_cast<int *>(self + 0x1bb0) = 0;
        *reinterpret_cast<int *>(self + 0x1bac) = 0;
        *reinterpret_cast<int *>(self + 0x1bcc) = 0;
        *reinterpret_cast<int *>(self + 0x1bc8) = 0;
    }
}

