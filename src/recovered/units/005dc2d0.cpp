// ORIGINAL: 0x005DC2D0 FILE
// name      ?get_special@@YAHPADPAH@Z
// size      60 bytes
// spans     0x005DC2D0-0x005DC30C
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
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
// address        0x005DC2D0
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005dc2d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_special@@YAHPADPAH@Z  at 0x005DC2D0  (60 bytes)
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

struct int8;

// Mangled `PADPAH` decodes plainly to (char*, int*); the scaffold's opaque
// `struct int8` forward-decl is IDA's guess, not the mangled ground truth.
//
// NEAR MISS, not byte-exact. Ruled out, all confirmed by direct
// recompilation across all four measured flag sets:
//   - Loop shape: `do { c = *p; if (special) break; ++p; } while (--n);`
//     (fused decrement-and-test) reproduces the ORIGINAL's single
//     dec/jne-tail loop with no rotation. The moment `--n` is written as
//     its own statement before a separate `while (n != 0)`, the backend
//     rotates the loop and duplicates the character reload plus an extra
//     `jmp` - a 059-vs-052-byte, one-`jmp` difference that persists
//     through `break`, `goto label`, and a plain nested `if`, so it is
//     the fused-decrement spelling that matters, not the exit mechanism.
//   - Return-value merge: the original does a REDUNDANT round trip -
//     `mov [ebp+8], eax` after the loop, then a SHARED `mov eax,[ebp+8]`
//     at the join with the early (`n==0`) exit, which already had the
//     right value in eax and needed no reload. Every source form that
//     reproduces this (reassigning the scan pointer back into `a1`,
//     into a fresh `result` local, single vs. two `return` statements)
//     brings back the loop rotation above - the two byte-costs appear
//     ONLY together, never the round trip alone, across all four flag
//     sets. Whatever produces the plain non-rotated loop keeps the
//     pointer in eax across the join with no stack traffic (52-57 bytes,
//     8 short of the original's 60).
//   - Register choice tracks the same pair: the clean 52-byte shape picks
//     esi to hold `a2`; the original spends ebx. Never isolated the
//     source shape that flips it independent of the above.
// Best reached: MISMATCH, 59/60 bytes, one edit (an inserted `jmp` from
// the rotation), mnemonic similarity 0.833.
int __cdecl get_special(char *a1, int *a2) {
    int n = *a2;
    if (n != 0) {
        char *p = a1;
        do {
            char c = *p;
            if (c == '{' || c == '}' || c == '[' || c == ']' || c == '$') {
                break;
            }
            ++p;
        } while (--n);
        *a2 = n;
        a1 = p;
    }
    return reinterpret_cast<int>(a1);
}
