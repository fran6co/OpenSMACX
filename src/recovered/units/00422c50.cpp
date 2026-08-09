// ORIGINAL: 0x00422C50 FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00422C50
// name           ?clear@BattleWin@@QAEXXZ
// size           101 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00422c50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?clear@BattleWin@@QAEXXZ  at 0x00422C50  (101 bytes)
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

struct BoxSpriteParams;
struct RECT;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Buffer { public:
    int box_sprite(RECT*, BoxSpriteParams*);
};
class GraphicWin { public:
    void soft_update();
};
class MainInterface { public:
    void redraw_complete();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;

class BattleWin { public:
    void clear();
};

void BattleWin::clear() {
    // Reach fields by offset - the class is deliberately empty.
    char *self = reinterpret_cast<char *>(this);
    Buffer *buffer = reinterpret_cast<Buffer *>(g_007aec64);
    BoxSpriteParams *params = reinterpret_cast<BoxSpriteParams *>(g_0078d528);

    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0x30), params);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0x40), params);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0x50), params);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0x60), params);

    reinterpret_cast<MainInterface *>(g_007ae820)->redraw_complete();
    reinterpret_cast<GraphicWin *>(g_007ae820)->soft_update();
}
