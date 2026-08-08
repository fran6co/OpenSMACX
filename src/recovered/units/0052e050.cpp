// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0052E050
// name           ?hang_up@NetDaemon@@QAEXXZ
// size           257 bytes
// measured tier  MISMATCH
// divergence     16
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052e050/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hang_up@NetDaemon@@QAEXXZ  at 0x0052E050  (257 bytes)
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
class NetMsg { public:
    void close();
};
class PlayerLock { public:
    void clear();
};
void __cdecl message_data(int, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_009a681c = (int *)0x009A681C;
static int *const g_009a6820 = (int *)0x009A6820;

class NetDaemon { public:
    void hang_up();
};

void NetDaemon::hang_up() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x1b3c) != 0) {
        return;
    }
    if (*g_0093f660 == 0) {
        return;
    }
    *reinterpret_cast<int *>(self + 0x1b3c) = 1;
    if (*reinterpret_cast<unsigned char *>(g_009a681c) & 0x10) {
        if (*g_0093a938 == 0) {
            if (*g_009a6820 != *g_00939284) {
                goto L0cc;
            }
        } else if (*g_0093e8c0 == 0) {
            goto L145;
        }
        message_data(0x8301, 0, 0, 0, 0, 0);
    }
L0cc:
    if (*g_0093e8c0 != 0) {
        reinterpret_cast<PlayerLock *>(self + *g_00939284 * 28 + 0x14a0)->clear();
        if (*reinterpret_cast<int *>(self + 0x1580) == *g_00939284) {
            *reinterpret_cast<int *>(self + 0x1584) = 0;
            *reinterpret_cast<int *>(self + 0x1580) = 0;
        }
        *reinterpret_cast<unsigned int *>(self + 0x1b7c) &=
            ~*reinterpret_cast<unsigned int *>(self + 0x1b80 + *g_00939284 * 4);
        message_data(0x4b00, 0, 0, 0, 0, 0);
    }
L145:
    reinterpret_cast<NetMsg *>(g_00805338)->close();
}
