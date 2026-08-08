// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00407E80
// name           ?on_listbox_scroll_create@AlphaSave@@QAEHXZ
// size           74 bytes
// measured tier  NO_COMPILE
// refusal        u00407e80.cpp(45) : error C2061: syntax error : identifier 'Win' u00407e80.cpp(51) : error C2065: 'GraphicWin' : undeclared identifier u00407e80.cpp(51) : error
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00407e80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_listbox_scroll_create@AlphaSave@@QAEHXZ  at 0x00407E80  (74 bytes)
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
    int init(int, int, int, int, Win*, int, int);
};
class Win { public:
    int is_visible();
};
void __cdecl scroll_normal();
void __cdecl scroll_show(GraphicWin*, Scroll*);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 2
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
};

class AlphaSave { public:
    int on_listbox_scroll_create();
};

int AlphaSave::on_listbox_scroll_create() {
    scroll_normal();
    Scroll *scroll = reinterpret_cast<Scroll *>(reinterpret_cast<char *>(this) + 0x3dd8);
    scroll->init(0x147, 0x15, 0x18, 0xd8, reinterpret_cast<Win *>(this), -1, 1);
    reinterpret_cast<VCall *>(scroll)->slot002();
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        scroll_show(reinterpret_cast<GraphicWin *>(this), scroll);
    }
    return reinterpret_cast<int>(scroll);
}
