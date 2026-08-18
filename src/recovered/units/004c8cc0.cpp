// ORIGINAL: 0x004C8CC0 ??0VoiceTx@@QAE@XZ 0x004C8CC0-0x004C8DAF;0x004C8450-0x004C8457;0x00659FBE-0x00659FD8 FILE
// size      272 bytes
// prototype void (__thiscall ??0VoiceTx@@QAE@XZ)(VoiceTx* this)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C61E0 0x00645460 0x0064558A 0x006465F0
// indirect  0x004C8D44
// PRESERVED UNIT - measured SHARED_TAIL.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004C8CC0
// measured tier  SHARED_TAIL
// refusal        1 span(s) are COMDAT-folded and claimed by another function; no per-function verdict is well defined
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c8cc0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0VoiceTx@@QAE@XZ  at 0x004C8CC0  (272 bytes)
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

class Sound;
class VoiceTx;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Sound { public:
    void set_type(unsigned int);
};
extern "C" char *strcpy(char *, const char *);
extern "C" void *memset(void *, int, unsigned int);
int __cdecl fn_0064558a();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0
class VCall { public:
    virtual void slot000();  // <-- used
};

// Second shim for the same slot: the call site pushes one argument
// (`push 0x3e8` before `call [edx]`) but VCall::slot000 is nullary, so
// a second shim with the right arity is needed rather than an edit to
// the first (both are shims for the same reason; only one is used
// here). Same technique already proven at 0x004C8960 (VoiceRx).
class SlotCall { public:
    virtual void slot0(int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00659fce = (int *)0x00659FCE;
static int *const g_0066e3c0 = (int *)0x0066E3C0;
static int *const g_0066e444 = (int *)0x0066E444;
static int *const g_0066e8c4 = (int *)0x0066E8C4;
static int *const g_006792a0 = (int *)0x006792A0;
static int *const g_00687b30 = (int *)0x00687B30;
static int *const g_00687b34 = (int *)0x00687B34;
static int *const g_00687b38 = (int *)0x00687B38;

class VoiceTx { public:
    uint8_t pad_0_[0x3C];
    uint32_t field_3c_;

    VoiceTx();
};

// WALL: COMDAT-folded span. `verify_recovered_function.py` refuses this
// address before ever compiling a candidate - "1 span(s) are COMDAT-folded
// and claimed by another function; no per-function verdict is well
// defined" - because the leading helper at 0x004C8450
// (`mov dword ptr [ecx],0x66e444; ret`, printed above the main
// disassembly and the target of the unwind funclet at 0x00659FBE) is
// folded together with an identical thunk owned by another catalogued
// function (0x004C8960's VoiceRx uses the very same helper and hits the
// identical wall). There is no tier this body can reach, BYTE_EXACT or
// otherwise, no matter what is written here - this is a faithful
// straight-line reconstruction for coverage, not a scored one. It does
// not attempt the SEH frame (two unwind states - reset-vtable-only at
// 0x659fbe, then full `Sound::~Sound` at 0x659fc6 - over an opaque
// VoiceTx with no base class the scaffold lets this file add), the same
// shape already ruled out at VoiceRx and at 0x004043D0's ~UV2Player.
VoiceTx::VoiceTx() {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self) = reinterpret_cast<int>(g_0066e444);
    *reinterpret_cast<int *>(self + 4) = 0x7f;
    *reinterpret_cast<int *>(self + 8) = 0;
    memset(self + 0xc, 0, 0x24);
    *reinterpret_cast<int *>(self + 0x30) = 0;
    memset(self + 0x40, 0, 4);
    *reinterpret_cast<int *>(self) = reinterpret_cast<int>(g_0066e3c0);
    *reinterpret_cast<int *>(self + 0x44) = 0;
    *reinterpret_cast<int *>(self + 0x48) = 0;
    *reinterpret_cast<int *>(self + 0x3c) = 0;
    *reinterpret_cast<int *>(self + 0x4c) = 0;
    *reinterpret_cast<int *>(self + 0x38) = 0x3e8;
    int *flagsPtr = reinterpret_cast<int *>(self + 0x40);
    *flagsPtr = *flagsPtr & ~1;
    int handler = *reinterpret_cast<int *>(self + 0x3c);
    if (handler != 0) {
        // slot0(int): VCall::slot000 above is nullary but this call site
        // pushes one argument (`push 0x3e8` before `call [edx]`), so the
        // second shim SlotCall is used instead - same technique proven at
        // VoiceRx.
        reinterpret_cast<SlotCall *>(handler)->slot0(0x3e8);
    }
    *reinterpret_cast<int *>(self + 0x50) = 0;
    *reinterpret_cast<int *>(self) = reinterpret_cast<int>(g_0066e8c4);
    memset(self + 0x54, 0, 4);

    // "Voice Tx\0" (9 bytes) copied out of the fixed string at g_00687b30
    // in three register-sized chunks (4+4+1), matching the disassembly's
    // `mov ecx,[0x687b30]` / `mov edx,[0x687b34]` / `mov al,byte[0x687b38]`.
    struct Str9 { int a; int b; char c; };
    Str9 buf;
    buf.a = *reinterpret_cast<int *>(g_00687b30);
    buf.b = *reinterpret_cast<int *>(g_00687b34);
    buf.c = *reinterpret_cast<char *>(g_00687b38);

    void *newBuf = ::operator new(0xa);
    *reinterpret_cast<void **>(self + 0x4c) = newBuf;
    strcpy(reinterpret_cast<char *>(newBuf), reinterpret_cast<char *>(&buf));

    reinterpret_cast<Sound *>(self)->set_type(7);
}
