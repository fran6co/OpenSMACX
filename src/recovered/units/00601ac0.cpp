// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00601AC0
// name           ?set_height@BasePop@@QAEXH@Z
// size           87 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00601ac0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_height@BasePop@@QAEXH@Z  at 0x00601AC0  (87 bytes)
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
static int *const g_009b7b1c = (int *)0x009B7B1C;

class BasePop { public:
    void set_height(int);
};

// field_a14 (a pointer) and bit 0x400 of field_30a8 gate an "auto-scale"
// path taken only when both are clear AND the global at 0x9b7b1c is
// >= 0x400: the stored height becomes (a1*3)/2 instead of a1 verbatim.
// Both branches independently re-derive the store target through
// field_21d0->[+8] (a per-instance slot-table offset added onto
// this+0x2200) rather than sharing that computation after the
// if/else - written as two full, separate tails here because a SHARED
// tail computation, appended once after the branch, gets register-
// allocated differently per branch by the compiler and lands one
// register swap short of BYTE_EXACT; duplicating the tail exactly as
// the original does was what closed it.
void BasePop::set_height(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<void **>(self + 0xa14) == 0 &&
        !(*reinterpret_cast<int *>(self + 0x30a8) & 0x400) &&
        *g_009b7b1c >= 0x400) {
        int val = (a1 * 3) / 2;
        void *base = *reinterpret_cast<void **>(self + 0x21d0);
        int off = *reinterpret_cast<int *>(reinterpret_cast<char *>(base) + 8);
        *reinterpret_cast<int *>(self + 0x2200 + off) = val;
        return;
    }
    void *base = *reinterpret_cast<void **>(self + 0x21d0);
    int off = *reinterpret_cast<int *>(reinterpret_cast<char *>(base) + 8);
    *reinterpret_cast<int *>(self + 0x2200 + off) = a1;
}
