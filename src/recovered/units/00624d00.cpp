// ORIGINAL: 0x00624D00 FILE
// name      sub_624d00
// size      55 bytes
// spans     0x00624D00-0x00624D37
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00622200
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00624D00
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00624d00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_624d00  at 0x00624D00  (55 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
int __stdcall sub_622200(int, int, int, int, int, int, int, int);

void __stdcall sub_624d00(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9) {
    // Forwards a1..a8 to sub_622200 iff a9 == -1. Return type changed from
    // the scaffold's `int` to `void`: the original never touches eax before
    // `ret 0x24` on either path (no `xor eax,eax` on the skip branch, no
    // `mov eax, ...` after the call), so nothing here returns a value.
    //
    // NEAR MISS, not byte-exact. Ruled out (see agent report): every
    // plain-forwarding source form this compiler is given for an N-argument
    // (N=4..8) __stdcall/__cdecl forward cycles THREE scratch registers
    // (eax/ecx/edx) while reading arguments two-ahead-of-the-next-push; the
    // original cycles only TWO (eax/edx), one-ahead. Confirmed with isolated
    // probes outside this function entirely (varying callee convention and
    // arity from 4 to 8) - the 3-register shape is fixed by this toolchain's
    // instruction scheduler for this call shape, not by anything in this
    // function's source. First divergence: index 4, original 'push' vs a
    // 'mov' the extra look-ahead register produces instead.
    if (a9 == -1) {
        sub_622200(a1, a2, a3, a4, a5, a6, a7, a8);
    }
}
