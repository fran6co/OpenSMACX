// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004090C0
// name           ?UNK1@BaseWin@@QAEHH@Z
// size           58 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004090c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK1@BaseWin@@QAEHH@Z  at 0x004090C0  (58 bytes)
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
static int *const g_0066a1a0 = (int *)0x0066A1A0;
static int *const g_0066a1f4 = (int *)0x0066A1F4;
static int *const g_0066efbc = (int *)0x0066EFBC;
static int *const g_0066f440 = (int *)0x0066F440;

class BaseWin { public:
    int UNK1(int);
};

// The address itself does work here (both tables are indexed, not just
// dereferenced once), and /O2 notices the two static-const pointers above
// are a constant 0x484 apart and rewrites the loop around a single walking
// pointer, dropping the original's independent scaled-index addressing.
// Declaring the two tables this body indexes as plain externs hides that
// constant relationship from the optimizer (the relocation is masked by
// the comparison, so the substitution costs nothing) and reproduces the
// original's per-array SIB addressing.
extern int g_a1a0_tbl[];
extern int g_a1f4_tbl[];
extern int g_efbc_tbl[];
extern int g_f440_tbl[];

int BaseWin::UNK1(int a1) {
    int i;
    for (i = 0; i < 0x15; i++) {
        if (g_a1a0_tbl[a1] == g_efbc_tbl[i] && g_a1f4_tbl[a1] == g_f440_tbl[i]) {
            return i;
        }
    }
    return 0;
}
