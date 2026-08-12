// ORIGINAL: 0x00406330 BYTE_EXACT FILE
// name      ?on_listbox_scroll_delete@Popup@@QAEHPAUScroll@@@Z
// size      76 bytes
// spans     0x00406330-0x0040637C
// prototype int (__thiscall ?on_listbox_scroll_delete@Popup@@QAEHPAUScroll@@@Z)(Popup* this, Scroll*)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045E540 0x005F7E90
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00406330
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00406330/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_listbox_scroll_delete@Popup@@QAEHPAUScroll@@@Z  at 0x00406330  (76 bytes)
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

struct Scroll;
// The scaffold's own callee line below names `GraphicWin*` without ever
// declaring it - a standalone compile of this file (no fresh scaffolding
// regen) fails C2065 on it. Forward-declared here to make the file
// self-contained; not a redeclaration; nothing else names it.
class GraphicWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Win { public:
    int is_visible();
};
void __cdecl scroll_hide(GraphicWin*, Scroll*);

class Popup { public:
    int on_listbox_scroll_delete(Scroll*);
};

int Popup::on_listbox_scroll_delete(Scroll* a1) {
    char *self = reinterpret_cast<char *>(this);
    int v = *reinterpret_cast<int *>(self + 0x30ac);
    if (v / 10000 == 1000) {
        return (int)a1;
    }
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        scroll_hide(reinterpret_cast<GraphicWin *>(this), a1);
    }
    return 0;
}
