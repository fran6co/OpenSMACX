// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0052DB70
// name           sub_52db70
// size           44 bytes
// measured tier  MISMATCH
// divergence     14
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052db70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_52db70  at 0x0052DB70  (44 bytes)
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
static int *const g_009a681c = (int *)0x009A681C;
static int *const g_009a6820 = (int *)0x009A6820;

int __cdecl sub_52db70() {
    // NEAR MISS, recorded so the next agent does not repeat the search.
    // MISMATCH: #14 original 'ret' vs rebuilt 'mov' (44 vs 45 bytes,
    // mnemonic similarity 0.9677, exactly one inserted instruction).
    // The first two guards match byte-for-byte. The final comparison
    // is the divergence: original is
    //     mov ecx, [0x9a6820]; mov edx, [0x939284]; xor eax, eax;
    //     cmp ecx, edx; sete al; ret
    // (operands land in ecx/edx, eax is reserved for the flag from the
    // start, no final move). Every spelling of `return a == b;` tried
    // here instead evaluates the first operand into EAX (regardless of
    // textual/declaration order), needs a second scratch register for
    // the flag, and ends with an extra `mov eax, <flag>` before `ret` -
    // proven in isolation with zero preceding code, so it is not caused
    // by the two guards above. RULED OUT: direct `return a==b`, ternary,
    // `!=` with swapped return literals, `!(a^b)`, unsigned casts, an
    // out-of-line `static` compare helper (does not inline here), bool
    // return type, a `this`-only `__thiscall` member (dead `this` in
    // ecx does not get reused for the load). A mutable `int result = 0;
    // if (a == b) result = 1; return result;` DOES reproduce the
    // ecx/edx/eax-preclear register roles exactly, but then compiles to
    // `cmp; jne; mov eax,1; ret` (branch, not `sete`) - 48 bytes. No
    // form found gets both the register roles and the `sete` tail at
    // once. Left as the closest candidate (edit_count 1) rather than
    // reverted, per the "do not throw away a measured near miss" rule.
    if ((*reinterpret_cast<unsigned char *>(g_009a681c) & 0x10) == 0) {
        return 0;
    }
    if (*g_0093a938 != 0) {
        return 0;
    }
    return *g_009a6820 == *g_00939284;
}
