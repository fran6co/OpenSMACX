// ORIGINAL: 0x00437BA0 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00437BA0
// name           ?on_mouse_move@DesignWin@@QAEXHH@Z
// size           53 bytes
// measured tier  NO_COMPILE
// refusal        u00437ba0.cpp(52) : error C2065: 'Win' : undeclared identifier u00437ba0.cpp(52) : error C2355: 'this' : can only be referenced inside non-static member functio
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00437ba0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_mouse_move@DesignWin@@QAEXHH@Z  at 0x00437BA0  (53 bytes)
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

struct spotPos;
struct spotType;
struct xCoord;
struct yCoord;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Spot { public:
    int check(int xCoord, int yCoord, int* spotPos, int* spotType);
};
void __stdcall fn_005f8500(Win* this);

class DesignWin { public:
    void on_mouse_move(int xCoord, int yCoord);
};

// BLOCKED at the SCAFFOLD level, independent of this body: the live emitter
// (tools/emit_translation_unit.py, called fresh by verify_recovered_function.py
// via mizuchi_writeback.build_unit - NOT the stale opaque skeleton cached
// above) hoists a full `class Buffer { ... Spot spot_; ... };` BEFORE
// `class Spot`'s own definition appears later in the same generated unit.
// `Buffer::spot_` is a Spot BY VALUE, needing a complete type, and only the
// forward declaration is in scope at that point, so cl fails unconditionally
// with:
//     error C2079: 'spot_' uses undefined class 'Spot'
// Reproduced with a totally empty body (`{}`) and with a real candidate
// body alike - same line, same error, every time. This is a class-hoisting-
// order bug in the emitter, not something a body-only edit can route
// around, and tools/ is off limits here. Logged so the next agent does not
// re-spend a compile cycle discovering the same thing.
//
// Logical recovery below (once the scaffold compiles): DesignWin's actual
// layout (per the live emitter) is `Spot spot_;` at 0x12764, and
// `Win::clear_bubble_text()` is a plain instance method with zero explicit
// arguments - `this` in ecx, no stack setup, matching the two bare `call`s
// with nothing pushed in between them.
void DesignWin::on_mouse_move(int a1, int a2) {
    int spot_pos;
    int spot_type;
    reinterpret_cast<Win *>(this)->clear_bubble_text();
    reinterpret_cast<Win *>(this)->clear_bubble_text();
    spot_.check(a1, a2, &spot_pos, &spot_type);
}
