// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x006337A0
// name           ?close@CheckButton@@QAEXXZ
// size           54 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006337a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close@CheckButton@@QAEXXZ  at 0x006337A0  (54 bytes)
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
static int *const g_00697f20 = (int *)0x00697F20;
static int *const g_00697f24 = (int *)0x00697F24;

class CheckButton { public:
    void close();
};

// CheckButton derives from GraphicWin (tail-jmp to ?close@GraphicWin@@QAEXXZ
// with no `this` adjustment); the scaffold's CheckButton is deliberately
// base-less, so the base class is expressed as a same-address reinterpret
// rather than by re-declaring CheckButton itself.
class GraphicWin { public:
    void close();
};

void CheckButton::close() {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0xa14) = 0;
    *reinterpret_cast<int *>(self + 0xa18) = 0;
    *reinterpret_cast<int *>(self + 0xa1c) = 0;
    *reinterpret_cast<int *>(self + 0xa20) = 0;
    *reinterpret_cast<int *>(self + 0xa28) = *g_00697f24;
    *reinterpret_cast<int *>(self + 0xa24) = *g_00697f20;
    reinterpret_cast<GraphicWin *>(self)->close();
}
