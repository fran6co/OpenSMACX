// ORIGINAL: 0x00493C20 FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00493C20
// name           ?on_mousewheel@ProdPicker@@QAEXH@Z
// size           72 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00493c20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_mousewheel@ProdPicker@@QAEXH@Z  at 0x00493C20  (72 bytes)
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
class Scroll { public:
    void on_mousewheel_down(int);
    void on_mousewheel_up(int);
};

class ProdPicker { public:
    void on_mousewheel(int);
};

void ProdPicker::on_mousewheel(int a1) {
    // Scroll is embedded at offset 0x7120 - reached by two separate lea/mov
    // pairs in the disassembly, one per branch, because the source recomputes
    // it independently in each arm rather than hoisting it above the if.
    char *self = reinterpret_cast<char *>(this);
    if (a1 < 0) {
        a1 = -a1;
        if (a1 > 0) {
            Scroll *scroll = reinterpret_cast<Scroll *>(self + 0x7120);
            int n = a1;
            do {
                scroll->on_mousewheel_down(1);
            } while (--n != 0);
        }
    } else if (a1 > 0) {
        Scroll *scroll = reinterpret_cast<Scroll *>(self + 0x7120);
        int n = a1;
        do {
            scroll->on_mousewheel_up(1);
        } while (--n != 0);
    }
}
