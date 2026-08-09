// ORIGINAL: 0x004E2400 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004E2400
// name           sub_4e2400
// size           68 bytes
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e2400/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4e2400  at 0x004E2400  (68 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_0090e8e1 = (int *)0x0090E8E1;
static int *const g_0090e8e2 = (int *)0x0090E8E2;
static int *const g_0090e8e4 = (int *)0x0090E8E4;
static int *const g_0090e8e5 = (int *)0x0090E8E5;
static int *const g_0090e8e6 = (int *)0x0090E8E6;
static int *const g_0090e8e7 = (int *)0x0090E8E7;
static int *const g_0090e8e8 = (int *)0x0090E8E8;
static int *const g_0090e8e9 = (int *)0x0090E8E9;
static int *const g_0090e8ea = (int *)0x0090E8EA;
static int *const g_0090e8eb = (int *)0x0090E8EB;

// NOT BYTE_EXACT - MISMATCH, closest reached (10/68 bytes differ,
// mnemonic_similarity 0.857). This is 11 independent byte stores to a
// fixed-address config block; the original hoists the two repeated values
// (1, used 6x, into al; 0, used 2x, into cl) ahead of EVERY store,
// including the very first one at 0x90e8e0 - but that first store's value
// (0xff) is used only twice (0x90e8e0 and 0x90e8eb) and is left as a plain
// immediate both times rather than hoisted a third time. Spelling the two
// 0xff stores differently (`-1` through a signed char* vs `0xff` through
// an unsigned char*) stops the compiler from recognising them as the same
// constant and hoisting a third register - that alone gets every
// instruction to match except one thing: the original schedules the
// al/cl setup BEFORE the first (0x90e8e0) store; every source form tried
// here (plain sequential stores, and named `unsigned char one=1,zero=0;`
// locals declared first) instead schedules that setup right before its
// first REAL use, after the 0x90e8e0 store. Both orderings were tried
// with both the shared-register and split-literal spellings; the ordering
// never moved. This reads as the backend's own scheduling choice for
// where a dependency-free register load issues - the same class of thing
// the scheduler notes above describe for `lea`/`push`.
extern "C" void __cdecl sub_4e2400() {
    *reinterpret_cast<signed char *>(g_0090e8e0) = -1;
    *reinterpret_cast<unsigned char *>(g_0090e8e1) = 4;
    *reinterpret_cast<unsigned char *>(g_0090e8e2) = 1;
    *reinterpret_cast<unsigned char *>(g_0090e8e4) = 0;
    *reinterpret_cast<unsigned char *>(g_0090e8e5) = 0;
    *reinterpret_cast<unsigned char *>(g_0090e8e6) = 1;
    *reinterpret_cast<unsigned char *>(g_0090e8e7) = 1;
    *reinterpret_cast<unsigned char *>(g_0090e8e8) = 1;
    *reinterpret_cast<unsigned char *>(g_0090e8e9) = 1;
    *reinterpret_cast<unsigned char *>(g_0090e8ea) = 1;
    *reinterpret_cast<unsigned char *>(g_0090e8eb) = 0xff;
}
