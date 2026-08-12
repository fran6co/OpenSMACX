// ORIGINAL: 0x005D56B0 BYTE_EXACT FILE
// name      ?update@GraphicWin@@QAEXPAUGraphicWin@@@Z
// size      109 bytes
// spans     0x005D56B0-0x005D571D
// prototype void (__thiscall ?update@GraphicWin@@QAEXPAUGraphicWin@@@Z)(GraphicWin* this, GraphicWin*)
// callers   8   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ECFE0 0x005EFD20 0x005F74A0 0x005F7E90
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005D56B0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005d56b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?update@GraphicWin@@QAEXPAUGraphicWin@@@Z  at 0x005D56B0  (109 bytes)
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

// The Windows RECT layout - a shadow struct, not an opaque shell, because
// the body needs named fields. Forward-declared opaque by the emitter; the
// full definition just completes it.
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class Win { public:
    int is_visible();
    int update_window(RECT*);
    void client_to_screen(RECT*);
    // `?flip@Win@@QAAXPAURECT@@@Z` mangles `QAA`, not `QAE` - a __cdecl
    // (caller-pop) member, not __thiscall, matching the `add esp,4` after
    // the call and the absence of a `this` reload before it. The emitter's
    // fallback guessed a free function taking an extra `Win*`; static is
    // what the mangling actually says.
    static void flip(RECT*);
};

class GraphicWin { public:
    void update(GraphicWin*);
};

void GraphicWin::update(GraphicWin* a1) {
    // Reach fields by offset - the class is deliberately empty.
    char *self = reinterpret_cast<char *>(this);
    int diff1 = *reinterpret_cast<int *>(self + 0x480) -
                *reinterpret_cast<int *>(self + 0x478);
    int diff2 = *reinterpret_cast<int *>(self + 0x47c) -
                *reinterpret_cast<int *>(self + 0x474);

    if (reinterpret_cast<Win *>(this)->is_visible()) {
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = diff2;
        rect.bottom = diff1;
        reinterpret_cast<Win *>(this)->update_window(&rect);
        reinterpret_cast<Win *>(this)->client_to_screen(&rect);
        Win::flip(&rect);
    }
}
