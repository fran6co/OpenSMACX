// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0052DBA0
// name           ?current_server@@YAHXZ
// size           70 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052dba0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?current_server@@YAHXZ  at 0x0052DBA0  (70 bytes)
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
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_009a681c = (int *)0x009A681C;
static int *const g_009a6820 = (int *)0x009A6820;

int __cdecl current_server() {
    // MISMATCH, closest reached: 61/70 bytes, matches through mnemonic #5
    // (both loads, the `and`, and both outer guard branches are identical).
    // Divergence is instruction #6 on: the original spends TWO push/pop
    // pairs (esi, edi) and a xor/sete/test dance to compare g3 against g4
    // before restoring registers and testing the boolean; every source
    // spelling tried here (direct `==`, named locals for one/both operands,
    // top-level vs nested loads, goto-style vs nested-if for the shared
    // "not active" tail) instead gets a single push/pop esi plus a direct
    // cmp+jne under all four measured flag sets - a strictly BETTER/shorter
    // encoding the register allocator prefers once it is allowed to. Ruled
    // out: this is not a guard-duplication or tail-sharing issue (those are
    // fixed - see the block below matches through the merged trailing
    // return); it looks like a register-allocation choice (5 live values -
    // flagBit, inTransit, g3, g4, the sete result - forcing 2 callee-saved
    // spills) that none of the 4 available (/O1,/O2 x framed,frameless)
    // flag combinations reproduce from equivalent C++.
    int inTransit = *g_0093a938;
    if ((*g_009a681c & 0x10) != 0) {
        if (inTransit == 0) {
            if (*g_009a6820 == *g_00939284) {
                return 1;
            }
        }
    }
    if ((*g_009a681c & 0x10) != 0) {
        if (inTransit == 0) {
            return 0;
        }
    }
    return *g_0093e8c0;
}
