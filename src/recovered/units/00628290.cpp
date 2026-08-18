// ORIGINAL: 0x00628290 sub_628290 0x00628290-0x006282D1 FILE
// size      65 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00628290
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00628290/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_628290  at 0x00628290  (65 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
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

// Three stack floats (esp+4/+8/+0xc), x87 `fld`/`fmul dword ptr` throughout
// (single precision, never promoted to double), plain `ret` (cdecl, no
// callee-pop): this is `void (const float*, const float*, float*)`, a 3D
// cross product (a3 = a1 x a2), not the placeholder `int()`.
//
// NEAR MISS, not byte-exact. The arithmetic and the field/parameter mapping
// are right - Ghidra's decompile confirms the formula and every operand -
// but the FPU SCHEDULING does not reproduce. The original loads both
// "outer" factors of a term up front (`fld` the second product's left
// operand, `fld` the first product's left operand), multiplies the top by
// the first product's right operand, `fxch`, multiplies the (now-top)
// second operand by the second product's right operand, then `fsubp`: 2
// `fld` + 2 `fmul` + 1 `fxch` per component, one extra instruction each.
// Every measured flag set (/O1, /O2, framed and frameless) instead finds a
// pairing that needs no swap at all - loading whichever operand of each
// product makes the two results land in the right stack slots already -
// which is 3 fewer bytes per component (59-60 vs 65 total). Multiplication
// is commutative, so this triggered regardless of the syntactic operand
// order written (`a*b-c*d` and `b*a-d*c` compile identically), regardless
// of whether the four scalars were re-read from memory each time or hoisted
// into named float locals first, and regardless of pointer- vs struct-typed
// (Vec3 x/y/z) parameter access. `/Op`, `/Og-`, `/Od` and `/Ox` were also
// tried directly (outside the four measured sets `verify_recovered_
// function.py` scores against): `/Od` drops to a much larger 96-byte form
// that reloads every parameter pointer before each `fld`/`fmul` pair and
// still never emits the original's specific swap-based pairing. This reads
// as a fixed x87 instruction-selection choice tied to a compiler/flag
// combination not in the set this harness can reach, not a source-form
// problem - the WALLS section's "several distinct source spellings all
// produce the same wrong order" shape, here for FPU operand pairing rather
// than a `lea`/`push`.
// Best reached: MISMATCH, 59/65 bytes, mnemonic similarity 0.809, the same
// one-`fxch`-short edit repeated identically at all three components.
void __cdecl sub_628290(const float *a1, const float *a2, float *a3) {
    a3[0] = a1[1] * a2[2] - a2[1] * a1[2];
    a3[1] = a2[0] * a1[2] - a1[0] * a2[2];
    a3[2] = a1[0] * a2[1] - a2[0] * a1[1];
}
