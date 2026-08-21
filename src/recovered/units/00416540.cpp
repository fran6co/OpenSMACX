// ORIGINAL: 0x00416540 ?bubble_me_elmo2@BaseWin@@QAEXPADHH@Z 0x00416540-0x00416580 FILE BYTE_EXACT
// symbol    ?bubble_me_elmo2@BaseWin@@QAEXPAUint8@@HH@Z
// size      64 bytes
// prototype void (__thiscall ?bubble_me_elmo2@BaseWin@@QAEXPADHH@Z)(BaseWin* this, int8*, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240 0x005F83D0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00416540
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00416540/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?bubble_me_elmo2@BaseWin@@QAEXPADHH@Z  at 0x00416540  (64 bytes)
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

struct RECT;
struct int8;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Win { public:
    void client_to_screen(int*, int*);
    void set_bubble_text(int8*, RECT*);
};

class BaseWin { public:
    void bubble_me_elmo2(int8*, int, int);
};

// RECT is opaque in this scaffold, so the {left,top,right,bottom} block is
// built as its own local (same layout) and reinterpret_cast at the call.
// Sequential field assignment (r.left=a2; r.right=a2; ...) reloads one
// register per statement and mis-schedules against the a1 load; an
// aggregate initializer keeps a2/a3 live in two registers at once and
// reproduces the original's exact interleaving - byte-exact.
struct BubbleRect { int left; int top; int right; int bottom; };

void BaseWin::bubble_me_elmo2(int8* a1, int a2, int a3) {
    reinterpret_cast<Win *>(this)->client_to_screen(&a2, &a3);
    BubbleRect rect = { a2, a3, a2, a3 };
    reinterpret_cast<Win *>(this)->set_bubble_text(a1, reinterpret_cast<RECT *>(&rect));
}
