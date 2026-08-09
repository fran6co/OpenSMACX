// ORIGINAL: 0x00453A20 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00453A20
// name           ?load_bases@@YAXXZ
// size           74 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00453a20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load_bases@@YAXXZ  at 0x00453A20  (74 bytes)
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
static int *const g_0076e590 = (int *)0x0076E590;
static int *const g_0076e890 = (int *)0x0076E890;

void __cdecl load_bases() {
    // MISMATCH: single-instruction divergence at mnemonic #3. The original
    // materialises the outer accumulator's zero with `xor ebx, ebx`; every
    // source-form tried here (declaration order, separate decl+assign,
    // for(;;)+break, swapped operand order in the lea expression, int vs
    // pointer typing for the walked pointer, `register` hints, and routing
    // the two table addresses through `extern`/indexed-array globals
    // instead of these literals) still lowers it to `mov ebx, 0`. `esi`'s
    // per-outer-iteration reset one loop level in compiles to `xor` either
    // way, so this is a backend instruction-selection choice for this one
    // slot, not a shape problem - 25/26 mnemonics agree (similarity 0.9615).
    int ebx = 0;
    char **eax = reinterpret_cast<char **>(g_0076e590);
    do {
        int esi = 0;
        do {
            int edi = 4;
            do {
                char *ecx = reinterpret_cast<char *>(0x78e978 + ebx + esi);
                int edx = 3;
                do {
                    *eax = ecx;
                    ecx += 0x2c;
                    eax++;
                    edx--;
                } while (edx != 0);
                esi += 0x84;
                edi--;
            } while (edi != 0);
        } while (esi < 0x420);
        ebx += 0x65c;
    } while (reinterpret_cast<int>(eax) < reinterpret_cast<int>(g_0076e890));
}
