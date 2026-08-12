// ORIGINAL: 0x00618560 BYTE_EXACT FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00618560
// name           ?UNK14@Caviar@@QAEXH@Z
// size           64 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00618560/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK14@Caviar@@QAEXH@Z  at 0x00618560  (64 bytes)
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
static int *const g_009bb478 = (int *)0x009BB478;

class Caviar { public:
    void UNK14(int);
};

void Caviar::UNK14(int a1) {
    // Each field write goes through a fresh `*g_009bb478` on purpose: the
    // original reloads that global before every group after the first
    // (4 separate `mov [0x9bb478]` in the disassembly, not 1), which is
    // what a compiler does when it cannot prove a write through `dst`
    // doesn't alias the global's own storage. Caching it in a local
    // collapses those reloads and undersizes the function (ruled out).
    int *dst = reinterpret_cast<int *>(a1);
    if (dst != 0 && *g_009bb478 != 0) {
        dst[0] = reinterpret_cast<int *>(*g_009bb478)[2];
        dst[2] = reinterpret_cast<int *>(*g_009bb478)[4] + 1;
        dst[1] = reinterpret_cast<int *>(*g_009bb478)[3];
        dst[3] = reinterpret_cast<int *>(*g_009bb478)[5] + 1;
    }
}
