// ORIGINAL: 0x00498110 FILE
// name      ?UNK1@ReportWin@@QAEXHHHH@Z
// size      100 bytes
// spans     0x00498110-0x00498174
// prototype void (__thiscall ?UNK1@ReportWin@@QAEXHHHH@Z)(ReportWin* this, int, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006059B0 0x00605D20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00498110
// measured tier  MISMATCH
// divergence     20
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00498110/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK1@ReportWin@@QAEXHHHH@Z  at 0x00498110  (100 bytes)
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

struct position;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Scroll { public:
    void set_pos(int position);
    void set_range(int, int);
};

class ReportWin { public:
    void UNK1(int, int, int, int);
};

void ReportWin::UNK1(int a1, int a2, int a3, int a4) {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0xC0FC) = a1;
    *reinterpret_cast<int *>(self + 0xC100) = a2;
    *reinterpret_cast<int *>(self + 0xC104) = a3;
    *reinterpret_cast<int *>(self + 0xC108) = a4;
    Scroll *scroll = reinterpret_cast<Scroll *>(self + 0xC10C);
    scroll->set_range(a2, a3);
    *reinterpret_cast<int *>(self + 0xCB4C) = *reinterpret_cast<int *>(self + 0xC108);
    scroll->set_pos(*reinterpret_cast<int *>(self + 0xC0FC));
    *reinterpret_cast<int *>(self + 0xE270) = 0;
}
