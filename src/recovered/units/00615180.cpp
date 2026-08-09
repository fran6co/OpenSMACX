// ORIGINAL: 0x00615180 FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00615180
// name           ?UNK1@EditBox@@QAEHPAUFont@@HHHPAUWin@@@Z
// size           86 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00615180/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK1@EditBox@@QAEHPAUFont@@HHHPAUWin@@@Z  at 0x00615180  (86 bytes)
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

struct Font;
struct Win;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Buffer { public:
    int set_font(Font*, Font*, Font*, Font*);
    int text_line_height();
};

class EditBox { public:
    int init(int, int, int, int, Win*, int);
    // `ret 0x18` cleans 6 stack dwords under thiscall, but the mangled
    // name above lists only 5 explicit params; the 6th is pushed by every
    // caller and never read here - same shape as sub_5ffe80's unused
    // leading parameter earlier in this pass. Added trailing `int` to
    // match the callee-pop count; see PROPOSALS in the report.
    int UNK1(Font*, int, int, int, Win*, int);
};

int EditBox::UNK1(Font* a1, int a2, int a3, int a4, Win* a5, int a6) {
    if (a1 == 0) {
        return 3;
    }
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    buf->set_font(a1, 0, 0, 0);
    int lineheight = buf->text_line_height();
    int height_arg = lineheight + 2;
    return init(a2, a3, a4, height_arg, a5, 0);
}
