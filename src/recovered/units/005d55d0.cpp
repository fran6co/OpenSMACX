// ORIGINAL: 0x005D55D0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005D55D0
// name           ?update@GraphicWin@@QAEXHHHHPAUGraphicWin@@@Z
// size           95 bytes
// measured tier  MISMATCH
// divergence     25
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005d55d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?update@GraphicWin@@QAEXHHHHPAUGraphicWin@@@Z  at 0x005D55D0  (95 bytes)
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

// A fresh brief shows RECT as a concrete, real struct (not opaque) and
// `flip` as an ordinary thiscall member of Win, not the free `__cdecl`
// shim this scaffold guessed - fixed to match.
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class Win { public:
    int is_visible();
    int update_window(RECT* lprcSrc2);
    void flip(RECT*);
    void client_to_screen(RECT*);
};

class GraphicWin { public:
    void update(int, int, int, int, GraphicWin*);
};

void GraphicWin::update(int a1, int a2, int a3, int a4, GraphicWin* a5) {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        RECT rect;
        rect.left = a1;
        rect.top = a2;
        rect.right = a1 + a3;
        rect.bottom = a2 + a4;
        reinterpret_cast<Win *>(this)->update_window(&rect);
        reinterpret_cast<Win *>(this)->client_to_screen(&rect);
        reinterpret_cast<Win *>(this)->flip(&rect);
    }
}
