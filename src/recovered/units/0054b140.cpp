// ORIGINAL: 0x0054B140 BYTE_EXACT FILE
// name      ?battle_routine@@YAXH@Z
// size      118 bytes
// spans     0x0054B140-0x0054B1B6
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0048AE90 0x0048AF20 0x0048AF30 0x0054AFA0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0054B140
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0054b140/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?battle_routine@@YAXH@Z  at 0x0054B140  (118 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class PlanWin { public:
    void add_line(int, int, int);
    void clear_lines();
    void on_redraw();
};
int __cdecl attack_from(int baseID, int factionID);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
// `g_0068f9f0` is an INDEXED TABLE BASE (`[edx*4 + 0x68f9f0]`) - the
// fixed-address-pointer spelling folds that address arithmetic away under
// /O2, so this one is `extern T[]` instead (measured elsewhere in this
// codebase: indexed table base `lea` became `add` under the pointer form).
extern int g_0068f9f0[];
static int *const g_00834d70 = (int *)0x00834D70;
static int *const g_00856dc0 = (int *)0x00856DC0;
static int *const g_0093f7bc = (int *)0x0093F7BC;
static int *const g_0093f810 = (int *)0x0093F810;

void __cdecl battle_routine(int a1) {
    int result1 = attack_from(a1, *g_0093f7bc);
    int result2 = attack_from(a1, *g_0093f810);

    reinterpret_cast<PlanWin *>(g_00834d70)->clear_lines();
    reinterpret_cast<PlanWin *>(g_00834d70)
        ->add_line(result1, a1, g_0068f9f0[*g_0093f7bc]);
    reinterpret_cast<PlanWin *>(g_00834d70)
        ->add_line(result2, a1, g_0068f9f0[*g_0093f810]);
    reinterpret_cast<PlanWin *>(g_00856dc0)->on_redraw();
}
