// ORIGINAL: 0x004C8960 FILE
// PRESERVED UNIT - measured SHARED_TAIL.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004C8960
// name           ??0VoiceRx@@QAE@XZ
// size           260 bytes
// measured tier  SHARED_TAIL
// refusal        1 span(s) are COMDAT-folded and claimed by another function; no per-function verdict is well defined
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c8960/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0VoiceRx@@QAE@XZ  at 0x004C8960  (260 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under BOTH cl 12.00.8168 and
// i686-w64-mingw32-g++ -std=c++11. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

struct SOUNDTYPE;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Sound { public:
    void set_type(SOUNDTYPE);
};
extern "C" int _strcpy();  // arity unknown
int __cdecl _memset();
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00659f9a = (int *)0x00659F9A;
static int *const g_0066e3c0 = (int *)0x0066E3C0;
static int *const g_0066e444 = (int *)0x0066E444;
static int *const g_0066e82c = (int *)0x0066E82C;
static int *const g_00679240 = (int *)0x00679240;
static int *const g_00687b24 = (int *)0x00687B24;
static int *const g_00687b28 = (int *)0x00687B28;
static int *const g_00687b2c = (int *)0x00687B2C;

class VoiceRx { public:
    VoiceRx();
};

// WALL: COMDAT-folded span. The verifier refuses this address before ever
// compiling a candidate - `?span_classes.shared? == 1` because the leading
// helper at 0x004C8450 (`mov dword ptr [ecx],0x66e444; ret`, printed above
// the main disassembly - it is the target of the unwind funclet at
// 0x00659F8A) is COMDAT-folded with an identical thunk claimed by another
// catalogued function. "1 span(s) are COMDAT-folded and claimed by another
// function; no per-function verdict is well defined" - there is no tier
// this body can reach, BYTE_EXACT or otherwise, no matter what is written
// here. This is a faithful straight-line reconstruction for coverage, not
// a scored one: it does not attempt the SEH frame (two unwind states -
// reset-vtable-only at 0x659f8a, then full `Sound::~Sound` at 0x659f92 -
// over an opaque VoiceRx with no base class the scaffold lets this file
// add, the same shape already ruled out at 0x004043D0's ~UV2Player).
// `?set_type@Sound@@QAEXI@Z` mangles to a plain `unsigned int` parameter
// (the `I` code) even though the scaffold guesses an incomplete
// `SOUNDTYPE` struct type for it - the call site only ever pushes the
// literal 6, never a struct - so `set_type(6)` is used directly against
// the scaffold's own declaration rather than completing that struct.
VoiceRx::VoiceRx() {
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
        // slot0(int): the shim declared above the class the scaffold gives
        // (`VCall`, slot000, nullary) cannot take this call's one argument
        // (`push 0x3e8` before `call [edx]`), so this is a second,
        // independent shim rather than an edit to that one - both are
        // shims for the same reason, only one is being called here.
        reinterpret_cast<SlotCall *>(handler)->slot0(0x3e8);
    }
    *reinterpret_cast<int *>(self + 0x50) = 0;
    *reinterpret_cast<int *>(self) = reinterpret_cast<int>(g_0066e82c);

    // "Voice Rx\0" (9 bytes) copied out of the fixed string at g_00687b24
    // in three register-sized chunks (4+4+1), matching the disassembly's
    // `mov ecx,[0x687b28]` / `mov eax,[0x687b24]` / `mov dl,byte[0x687b2c]`.
    struct Str9 { int a; int b; char c; };
    Str9 buf;
    buf.a = *reinterpret_cast<int *>(g_00687b24);
    buf.b = *reinterpret_cast<int *>(g_00687b28);
    buf.c = *reinterpret_cast<char *>(g_00687b2c);

    reinterpret_cast<Sound *>(self)->set_type(6);

    void *newBuf = ::operator new(0xa);
    *reinterpret_cast<void **>(self + 0x4c) = newBuf;
    strcpy(reinterpret_cast<char *>(newBuf), reinterpret_cast<char *>(&buf));
}
