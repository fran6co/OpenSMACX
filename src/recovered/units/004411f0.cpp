// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004411F0
// name           sub_4411f0
// size           81 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004411f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4411f0  at 0x004411F0  (81 bytes)
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
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

// Two stack ints, no `this` (ecx is loaded from the stack, never
// dereferenced) -> plain __cdecl, not a member. Neither leaves anything
// in eax before `pop ebp; ret`, so the original returns void, not int.
//
// Each pair (a1, a2) / (a2, a1) indexes a 144 (0x90)-byte record at a
// fixed base and zeroes its first four dwords - a symmetric relation
// table entry cleared both ways. Writing the two symmetric index
// expressions out fully (rather than hoisting each into a named temp)
// reproduces the original's register allocation exactly: VC6 keeps
// `esi` live across the shared subexpression instead of spilling
// differently the way a hoisted temp does.
extern "C" void __cdecl sub_4411f0(int a1, int a2) {
    *reinterpret_cast<int *>(0x7388D8 + (a2 + a1 * 8) * 144) = 0;
    *reinterpret_cast<int *>(0x7388DC + (a2 + a1 * 8) * 144) = 0;
    *reinterpret_cast<int *>(0x7388D8 + (a1 + a2 * 8) * 144) = 0;
    *reinterpret_cast<int *>(0x7388DC + (a1 + a2 * 8) * 144) = 0;
    *reinterpret_cast<int *>(0x7388E0 + (a2 + a1 * 8) * 144) = 0;
    *reinterpret_cast<int *>(0x7388E4 + (a2 + a1 * 8) * 144) = 0;
    *reinterpret_cast<int *>(0x7388E0 + (a1 + a2 * 8) * 144) = 0;
    *reinterpret_cast<int *>(0x7388E4 + (a1 + a2 * 8) * 144) = 0;
}
