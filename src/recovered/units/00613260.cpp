// ORIGINAL: 0x00613260 BYTE_EXACT FILE
// name      ?on_scroll_delete@Dialogs@@QAEHPAUScroll@@@Z
// size      63 bytes
// spans     0x00613260-0x0061329F
// prototype int (__thiscall ?on_scroll_delete@Dialogs@@QAEHPAUScroll@@@Z)(Dialogs* this, Scroll*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00613260
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00613260/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_scroll_delete@Dialogs@@QAEHPAUScroll@@@Z  at 0x00613260  (63 bytes)
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

// Vtable shim. The scaffold's own `VCall` (slot009, nullary) cannot be
// completed with the one-`int`-argument signature this call site needs -
// redeclaring the same class name with a different signature is a
// redefinition (C2011). Declared fresh here under a different name instead;
// only declaration order matters for a VCall shim, so this is equivalent.
class VCallShim2 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual int slot009(int);  // <-- used
};

class Dialogs { public:
    int on_scroll_delete(Scroll*);
};

// `mov eax,[ecx-8]; sub eax,2; jne` is the SWITCH shape (AGENTS note: a
// single-case switch loads, subtracts the case constant, and tests via
// `jne`), not `if (x == 2)`, which the compiler instead folds to a fused
// `cmp [mem], imm` - confirmed empirically, `cmp` at instruction #0 against
// an `if`, byte-exact after switching to `switch`.
int Dialogs::on_scroll_delete(Scroll* a1) {
    char *self = reinterpret_cast<char *>(this);
    switch (*reinterpret_cast<int *>(self - 8)) {
    case 2: {
        char *base = *reinterpret_cast<char **>(self - 0x188);
        int off = *reinterpret_cast<int *>(base + 4);
        char *mid = self + off - 0x140;
        unsigned char flags = *reinterpret_cast<unsigned char *>(mid + 0x50);
        if (flags & 0x40) {
            VCallShim2 *obj = *reinterpret_cast<VCallShim2 **>(mid + 0x7c);
            if (obj != 0) {
                return obj->slot009(reinterpret_cast<int>(a1));
            }
        }
    }
    }
    return reinterpret_cast<int>(a1);
}
