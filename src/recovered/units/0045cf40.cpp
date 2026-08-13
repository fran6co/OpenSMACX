// ORIGINAL: 0x0045CF40 BYTE_EXACT FILE
// name      ?on_iface_left_down@MainInterface@@QAEXHH@Z
// size      53 bytes
// spans     0x0045CF40-0x0045CF75
// prototype void (__thiscall ?on_iface_left_down@MainInterface@@QAEXHH@Z)(MainInterface* this, int xCoord, int yCoord)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005169F0 0x005FAB00
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045CF40
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045cf40/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_iface_left_down@MainInterface@@QAEXHH@Z  at 0x0045CF40  (53 bytes)
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

struct spotPos;
struct spotType;
struct xCoord;
struct yCoord;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Console { public:
    void end_my_turn();
};
class Spot { public:
    int check(int xCoord, int yCoord, int* spotPos, int* spotType);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009156b0 = (int *)0x009156B0;

class MainInterface { public:
    void on_iface_left_down(int xCoord, int yCoord);
};

// EMITTER BUG worked around: the generated head above declares each
// parameter with TWO identifiers (`int xCoord a1`), which is not valid
// C++ and does not compile. Rewritten below with the plain `a1`/`a2` names
// the collision-avoidance logic evidently intended - see the agent report.
void MainInterface::on_iface_left_down(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int result = reinterpret_cast<Spot *>(self + 0x24620)->check(a1, a2, &a2, 0);
    if (result != -1 && a2 == 5) {
        reinterpret_cast<Console *>(g_009156b0)->end_my_turn();
    }
}
