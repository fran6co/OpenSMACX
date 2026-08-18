// ORIGINAL: 0x004BDEA0 ?draw_arrow@TutWin@@QAEXXZ 0x004BDEA0-0x004BDF16 FILE BYTE_EXACT
// size      118 bytes
// prototype void (__thiscall ?draw_arrow@TutWin@@QAEXXZ)(TutWin* this)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F2AC0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004BDEA0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004bdea0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_arrow@TutWin@@QAEXXZ  at 0x004BDEA0  (118 bytes)
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
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

struct xCoord;
struct yCoord;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Win { public:
    void window_line_raw(int, int, int xCoord, int yCoord, int, int, unsigned int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068a5a0 = (int *)0x0068A5A0;

class TutWin { public:
    void draw_arrow();
};

void TutWin::draw_arrow() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x53A4) != 0) {
        (*reinterpret_cast<Win **>(self + 0x53D4))->window_line_raw(
            *reinterpret_cast<int *>(self + 0x538C),
            *reinterpret_cast<int *>(self + 0x5390),
            *reinterpret_cast<int *>(self + 0x5384),
            *reinterpret_cast<int *>(self + 0x5388),
            *g_0068a5a0,
            2, 2);
        int v = *reinterpret_cast<int *>(self + 0x539C);
        if (v >= 0) {
            (*reinterpret_cast<Win **>(self + 0x53D4))->window_line_raw(
                v,
                *reinterpret_cast<int *>(self + 0x53A0),
                *reinterpret_cast<int *>(self + 0x5394),
                *reinterpret_cast<int *>(self + 0x5398),
                *g_0068a5a0,
                2, 2);
        }
    }
}
