// ORIGINAL: 0x00497A60 BYTE_EXACT FILE
// name      ?on_left_double_click@ReportWin@@QAEXHH@Z
// size      82 bytes
// spans     0x00497A60-0x00497AB2
// prototype void (__thiscall ?on_left_double_click@ReportWin@@QAEXHH@Z)(ReportWin* this, int xCoord, int yCoord)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0041A920 0x005FAB00
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00497A60
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00497a60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_double_click@ReportWin@@QAEXHH@Z  at 0x00497A60  (82 bytes)
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

struct baseID;
struct spotPos;
struct spotType;
struct xCoord;
struct yCoord;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BaseWin { public:
    void zoom(int baseID, int);
};
class Spot { public:
    int check(int xCoord, int yCoord, int* spotPos, int* spotType);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_00885f40 = (int *)0x00885F40;

class ReportWin { public:
    void on_left_double_click(int xCoord, int yCoord);
};

// The address itself is the receiver for the zoom() call below, so an
// `extern` symbol is used instead of the const-pointer spelling: /O2 folds
// a literal address used only for its value, and the instruction that
// loads it into ecx as `this` vanishes with it.
extern BaseWin g_00497a60_baseWin;

void ReportWin::on_left_double_click(int a1, int a2) {
    Spot *spot = reinterpret_cast<Spot *>(reinterpret_cast<char *>(this) + 0x5994);
    int result = spot->check(a1, a2, &a2, &a1);
    if (result >= 0) {
        if (*g_00885f40 == 4) {
            if (a2 >= 0x400) {
                int t = a2 - 0x400;
                if (t < 0x400) {
                    g_00497a60_baseWin.zoom(t, 0);
                }
            }
        }
    }
}
