// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0046FC80
// name           ?UNK6@MapWin@@QAEXHHH@Z
// size           83 bytes
// measured tier  MISMATCH
// divergence     23
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0046fc80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK6@MapWin@@QAEXHHH@Z  at 0x0046FC80  (83 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
// g_0066efbc/g_0066f440/g_0066f8c8 are indexed table bases (the ADDRESS
// itself does work: `[reg*4 + ADDR]`), so the const-pointer spelling lets
// /O2 fold the two nearby literals (0x66F440 - 0x66EFBC = 0x484) into one
// walking pointer with an `add`, which the original never does - it keeps
// each table as its own SIB-indexed base. `extern T name[];` keeps the
// addresses unknown at compile time so that folding cannot happen; the
// relocation it needs is masked by the comparison.
extern int g_0066efbc[];
extern int g_0066f440[];
extern int g_0066f8c8[];
static int *const g_00949870 = (int *)0x00949870;

class MapWin { public:
    void UNK6(int, int, int);
};

void MapWin::UNK6(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int **>(self + 4) != 0) {
        int i = 0;
        int n = g_0066f8c8[a3];
        if (n > 0) {
            do {
                int idx = g_0066f440[i] * (*g_00949870 >> 1)
                        + (g_0066efbc[i] >> 1);
                i++;
                (*reinterpret_cast<int **>(self + 4))[idx * 3] = -1;
            } while (i < n);
        }
    }
}
