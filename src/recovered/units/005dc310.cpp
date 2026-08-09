// ORIGINAL: 0x005DC310 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005DC310
// name           ?get_special_link@@YAHPADPAH@Z
// size           65 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005dc310/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_special_link@@YAHPADPAH@Z  at 0x005DC310  (65 bytes)
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

// `struct int8;` above is the scaffolding's own placeholder type (no
// `int8` typedef is in scope in this unit, so `int8*` could not have
// compiled) - the catalogue's `int8` means plain `char` (see the note
// other units in this pass carry), so the parameter is spelled `char *`
// directly instead of redeclaring the missing typedef.
//
// RULED OUT, not reached: the original keeps a1's stack home ([ebp+8])
// live as a second copy of the cursor - `mov [ebp+8], eax` on the
// loop-exhausted/found path, `mov eax, [ebp+8]` at the single shared
// return - and dedicates a callee-saved `ebx` to `a2` for the whole
// body (`push ebx` in the prologue, matching the note above), while
// comparing through `cl`. None of the four measured flag sets
// reproduce both the register dedication AND a non-rotated
// `dec edx; jne` tail at once from any spelling tried here: forcing the
// stack round-trip (splitting the cursor into its own local reassigned
// back to `a1` at the end) gets the round-trip under /O1 but also
// rotates the loop (peels the first read, adds a `jmp` back into the
// middle) - not present in the original - which scores WORSE
// (mnemonic_similarity 0.8438 vs 0.8814 below). This is the better of
// the two: same loop shape as the original, just without the spill.
int __cdecl get_special_link(char *a1, int *a2) {
    int n = *a2;
    if (n != 0) {
        char c;
        do {
            c = *a1;
            if (c == '{' || c == '}' || c == '[' || c == ']' || c == '$' || c == '=') {
                break;
            }
            ++a1;
            --n;
        } while (n != 0);
        *a2 = n;
    }
    return reinterpret_cast<int>(a1);
}
