// ORIGINAL: 0x004BA6B0 BYTE_EXACT FILE
// name      ??0TutWin@@QAE@XZ
// size      110 bytes
// spans     0x004BA6B0-0x004BA71E
// prototype void (__thiscall ??0TutWin@@QAE@XZ)(TutWin* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004048A0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004BA6B0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004ba6b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0TutWin@@QAE@XZ  at 0x004BA6B0  (110 bytes)
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

class Popup;
class TutWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Popup { public:
    Popup();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066dd7c = (int *)0x0066DD7C;
static int *const g_0066dd84 = (int *)0x0066DD84;
static int *const g_008cc244 = (int *)0x008CC244;

// Inherits from Popup so the base constructor call is automatic (`call
// Popup::Popup()` with `this` unchanged, matching the target exactly) -
// but declares NO other members, and every field below is reached by raw
// offset from `this` instead. Popup's true size is only lower-bounded
// (docs/recovery/access-lower-bounds.csv), so letting TutWin's own named
// fields sit after it as ordinary members would place them at whatever
// offset the guessed base size implies, not the offset the target uses.
class TutWin : public Popup { public:
    TutWin();
};

TutWin::TutWin() {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<void **>(self) = g_0066dd84;
    *reinterpret_cast<void **>(self + 0x444) = g_0066dd7c;
    *reinterpret_cast<int *>(self + 0x53B4) = 0;
    *reinterpret_cast<int *>(self + 0x53C0) = 0;
    *reinterpret_cast<int *>(self + 0x53D0) = 0;
    *reinterpret_cast<int *>(self + 0x53D4) = 0;
    *reinterpret_cast<int *>(self + 0x53A4) = 0;
    *reinterpret_cast<int *>(self + 0x5380) = -1;
    *reinterpret_cast<int *>(self + 0x537C) = -1;
    *reinterpret_cast<int *>(self + 0x539C) = -1;
    *reinterpret_cast<int *>(self + 0x53A8) = 0;
    *reinterpret_cast<int *>(self + 0x53AC) = -1;
    *reinterpret_cast<int *>(self + 0x53B8) = 0;
    *reinterpret_cast<int *>(self + 0x53C4) = 0;
    *g_008cc244 = 0;
}

