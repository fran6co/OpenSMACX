// ORIGINAL: 0x005CE2B0 BYTE_EXACT FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005CE2B0
// name           sub_5ce2b0
// size           64 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ce2b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ce2b0  at 0x005CE2B0  (64 bytes)
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

typedef unsigned int SIZE_T;

// ---- callees, declared and never defined (a definition would be inlined) ----
// The live scaffolding respells this as `_beginthread` (one leading
// underscore in source) - cdecl decoration adds the second one, matching
// the disassembly's `call ... ; __beginthread`.
extern "C" int __cdecl _beginthread(int, SIZE_T, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005cdfa0 = (int *)0x005CDFA0;
static int *const g_006690f0 = (int *)0x006690F0;
static int *const g_00669100 = (int *)0x00669100;

// SIGNATURE PROPOSAL: `int __cdecl sub_5ce2b0()` (no receiver) does not
// match the disassembly - `mov esi, ecx` followed by `[esi+0x3b0]` and
// `[esi+0x3b4]` field access with zero stack parameters is a __thiscall
// receiver, not a nullary cdecl free function. `sub_5ce2b0` is not a
// mangled name (Ghidra could not resolve one), so nothing pins the
// convention; byte_match locates the compiled function by section, not by
// symbol name, so the class name below is free to invent.
typedef long (__stdcall *InterlockedExchangeFn)(long *, long);
typedef int (__stdcall *SetThreadPriorityFn)(int, int);

class ThreadStarter { public:
    int start();
};

int ThreadStarter::start() {
    char *self = reinterpret_cast<char *>(this);
    (*reinterpret_cast<InterlockedExchangeFn *>(g_00669100))(
        reinterpret_cast<long *>(self + 0x3b0), 1);
    int handle = _beginthread(reinterpret_cast<int>(g_005cdfa0), 0,
                               reinterpret_cast<int>(this));
    *reinterpret_cast<int *>(self + 0x3b4) = handle;
    (*reinterpret_cast<SetThreadPriorityFn *>(g_006690f0))(handle, 2);
    return *reinterpret_cast<int *>(self + 0x3b4) == 0;
}
