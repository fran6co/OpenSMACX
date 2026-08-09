// ORIGINAL: 0x005DE2E0 FILE
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005DE2E0
// name           sub_5de2e0
// size           58 bytes
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005de2e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5de2e0  at 0x005DE2E0  (58 bytes)
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
// The scaffold's guess (`LPCSTR lpString`) names an undeclared type and does
// not compile (C2065). `sub_5de320` is reached with a plain stack int (the
// pushed value is `eax` loaded straight from a stack dword, never treated as
// a pointer), so it is redeclared here with that arity/type instead.
int __stdcall sub_5de320(int a1);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b33cc = (int *)0x009B33CC;
static int *const g_009b3a90 = (int *)0x009B3A90;
static int *const g_009b3a94 = (int *)0x009B3A94;
static int *const g_009b3a98 = (int *)0x009B3A98;
static int *const g_009b3a9c = (int *)0x009B3A9C;

// `mov dword ptr [ecx + 0x18], eax` with no stack access for the receiver:
// ecx is `this`, so this is a __thiscall member, not the free __stdcall the
// scaffold guessed. The comparison is over the object's code rather than a
// symbol lookup, so the class is free to be introduced here.
class Sub5de2e0Target { public:
    int sub_5de2e0(int a1, int a2, int a3, int a4);
};

int Sub5de2e0Target::sub_5de2e0(int a1, int a2, int a3, int a4) {
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x18) = 0;
    *g_009b3a90 = a2;
    *g_009b3a9c = 0;
    *reinterpret_cast<char *>(g_009b33cc) = 0;
    *g_009b3a94 = a3;
    *g_009b3a98 = a4;
    return sub_5de320(a1);
}

// NEAR MISS, not byte-exact: MNEMONIC_ONLY, 57 of 58 bytes. Every mnemonic
// and the `ret 0x10` pop count already match, in order; the residual is pure
// register allocation, not a wrong operation. The original stores a2, a3 AND
// a4 all through EDX (`mov dword ptr [addr], edx`, 6 bytes each), reloading
// it fresh before each store, and keeps a1 in EAX alone for the trailing
// `push`/call - ECX is dead the instant the `this`-store finishes.
//
// This body's compile instead treats the position of each store, not its
// identity: whichever real store sits SECOND in program order gets hoisted
// to a load at the very top of the function into EDX; whichever sits FIRST
// gets ECX (recycling the just-freed `this` register); whichever sits THIRD
// gets EAX, which folds to the short `A3 moffs32` form (5 bytes) instead of
// the general 6-byte ModRM the original has there - the whole 58-vs-57 gap.
// Confirmed positional, not value-linked: swapping which of a2/a3/a4 occupy
// which of the three statement slots (7 source permutations tried,
// including grouping the two zero-stores first, pre-loading `a1` into a
// named local ahead of everything else, and giving `this`'s field a real
// class member instead of a pointer cast) always reproduces the identical
// ECX/EDX/EAX-short pattern, just relabelled - so the extra ECX use is a
// property of the STATEMENT SHAPE (three pass-through stores plus a
// terminal call), not of which source spelling names which value. Ruled
// out rather than found: this reads as the same class of backend scheduling
// artifact called out for `lea`/`push` ordering elsewhere, extended to
// register selection, and is left for whoever can compare against a build
// that pins VC6's register allocator directly.
