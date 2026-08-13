// ORIGINAL: 0x00630C30 FILE
// name      sub_630c30
// size      76 bytes
// spans     0x00630C30-0x00630C7C
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401D80
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00630C30
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00630c30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_630c30  at 0x00630C30  (76 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
int __stdcall sub_401d80(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009be4e8 = (int *)0x009BE4E8;

// CLOSEST REACHED - MISMATCH, not BYTE_EXACT. 76 original bytes vs 77
// rebuilt; single edit, insert at mnemonic #3: the compiler emits a
// redundant `xor eax,eax` right before the early `ret 0x10` even though
// eax already holds a1's value (0 in this branch) straight out of the
// preceding `test`. Tried and ruled out as the cause: `return 0;` vs
// `return a1;` (identical result); inlining every field read instead of
// naming locals (drops the register-pressure shape entirely, 69 bytes,
// still has the same redundant xor); a single shared-epilogue rewrite
// (`int result = a1; if (a1) {...; result = 1;} return result;`) - which
// also inverts the branch to `je` and loses 3 bytes elsewhere, confirming
// the two-separate-epilogues structure below is the right shape and the
// xor is the ONLY remaining gap. Left as the closest body reached.
extern "C" int __stdcall sub_630c30(int a1, int a2, int a3, int a4) {
    if (a1 == 0) {
        return 0;
    }
    char *p = reinterpret_cast<char *>(a1);
    int f34 = *reinterpret_cast<int *>(p + 0x34);
    int f4 = *reinterpret_cast<int *>(p + 4);
    int f30 = *reinterpret_cast<int *>(p + 0x30);
    int f2c = *reinterpret_cast<int *>(p + 0x2c);
    int f28 = *reinterpret_cast<int *>(p + 0x28);
    char *dst = reinterpret_cast<char *>(*g_009be4e8);
    *reinterpret_cast<int *>(dst + 0x1c) = f30;
    *reinterpret_cast<int *>(dst + 0x20) = a1 + 8;
    *reinterpret_cast<int *>(dst + 0x24) = f4;
    *reinterpret_cast<int *>(dst + 0x28) = f28;
    *reinterpret_cast<int *>(dst + 0x2c) = f2c;
    *reinterpret_cast<int *>(dst + 0x30) = f34;
    sub_401d80(0);
    return 1;
}
