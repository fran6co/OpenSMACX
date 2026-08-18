// ORIGINAL: 0x004E0210 ?editor_date@Console@@QAEXXZ 0x004E0210-0x004E0282 FILE
// size      114 bytes
// prototype void (__thiscall ?editor_date@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0041E790 0x00514880 0x005ABF20 0x005BFD10 0x005C89A0
// indirect  0x004E0268
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004E0210
// measured tier  MISMATCH
// divergence     22
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e0210/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?editor_date@Console@@QAEXXZ  at 0x004E0210  (114 bytes)
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
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BaseWin { public:
    void on_redraw();
};
int __cdecl X_pop_ask_number(char*, int, int (__cdecl *)());
int __cdecl game_year(int turn);
void __cdecl auto_undo();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 12
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006890f0 = (int *)0x006890F0;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_0094b4d0 = (int *)0x0094B4D0;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009bb598 = (int *)0x009BB598;

class Console { public:
    void update_data(int);
    void editor_date();
};

void Console::editor_date() {
    auto_undo();
    int result = X_pop_ask_number(reinterpret_cast<char *>(g_006890f0), game_year(*g_009a64d4), 0);
    if (result == 0) {
        *g_009a64d4 = *g_009bb598 - *g_0094b4d0;
        if (*g_009a64d4 < 1) {
            *g_009a64d4 = 1;
        }
        int p = *g_007d3c3c;
        int q = *reinterpret_cast<int *>(p);
        int r = *reinterpret_cast<int *>(q + 4);
        reinterpret_cast<VCall *>(r + p)->slot012();
        reinterpret_cast<BaseWin *>(g_006a7628)->on_redraw();
        reinterpret_cast<Console *>(g_009156b0)->update_data(0);
    }
}
