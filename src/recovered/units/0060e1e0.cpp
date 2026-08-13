// ORIGINAL: 0x0060E1E0 BYTE_EXACT FILE
// name      ?on_left_double_click@RadioButton@@QAEXHH@Z
// size      55 bytes
// spans     0x0060E1E0-0x0060E217
// prototype void (__thiscall ?on_left_double_click@RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609A50
// indirect  0x0060E20D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0060E1E0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0060e1e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_double_click@RadioButton@@QAEXHH@Z  at 0x0060E1E0  (55 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class Dialog { public:
    int get_selected_id();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7ab8 = (int *)0x009B7AB8;

class RadioButton { public:
    void on_left_double_click(int, int);
};

// `this - 0x18` recurs twice in the original: RadioButton is embedded (or
// derived) at offset 0x18 within an enclosing object that itself virtually
// inherits two bases - one holding the field this reads (its own field sits
// at 0xac from THAT base's start, but note the compiler folds the -0x18 into
// the field's own displacement instead of materialising the outer pointer,
// which is why the padding below is 0xc4, not 0xac: 0xc4-0x18=0xac is what
// surfaces literally in the instruction), the other being Dialog (used for
// the get_selected_id() call, non-virtual, dispatched on the SAME adjusted
// pointer). `this - 8` is a plain field of the enclosing object (no virtual
// base involved) holding a `void(__cdecl*)(int)` callback.
//
// Both `this-0x18` accesses and both `this-8` accesses are written with NO
// cached local: the original re-reads `[esi-0x18]` a second time before the
// `get_selected_id()` call instead of reusing the first load, and calls
// `dword ptr [esi-8]` directly rather than through the register it already
// used for the null check. Caching either into a named local lets the
// optimizer keep it live in a register and produces a smaller function that
// no longer matches (51 bytes vs the original's 55).
class FieldBase {
public:
    char pad[0xc4];
    int marker;
};

class Outer : public virtual FieldBase, public virtual Dialog {
};

typedef void (__cdecl *Callback)(int);

void RadioButton::on_left_double_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    *g_009b7ab8 = reinterpret_cast<Outer *>(self - 0x18)->marker;
    if (*reinterpret_cast<Callback *>(self - 8) != 0) {
        (*reinterpret_cast<Callback *>(self - 8))(
            reinterpret_cast<Outer *>(self - 0x18)->get_selected_id());
    }
}
