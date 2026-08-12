// ORIGINAL: 0x004C39D0 FILE
// name      ?show_all@WorldWin@@QAEXXZ
// size      105 bytes
// spans     0x004C39D0-0x004C3A39
// prototype void (__thiscall ?show_all@WorldWin@@QAEXXZ)(WorldWin* this)
// callers   5   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED9D0 0x0062B870
// indirect  0x004C39FD 0x004C3A29
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004C39D0
// measured tier  MISMATCH
// divergence     11
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c39d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?show_all@WorldWin@@QAEXXZ  at 0x004C39D0  (105 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class ButtonGroup { public:
    int set(int, int);
};
class Win { public:
    void show(int);
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1, 2. Split into two shims
// because slot 1 (0x4c39fd) takes one int and slot 2 (0x4c3a29) is
// nullary - the same class cannot spell both at the same index.
class VCallShow { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used
};
class VCallHide { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
};
// `hide()` is not on the emitter's Win (it only carries `show`), so a
// second opaque view of the same fixed object supplies it. The direct
// call's target bytes are relocated and masked either way.
class WinHide { public:
    void hide();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007cd12c = (int *)0x007CD12C;
static int *const g_007d3c40 = (int *)0x007D3C40;
static int *const g_008e9f60 = (int *)0x008E9F60;

class WorldWin { public:
    void show_all();
};

void WorldWin::show_all() {
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x239cc) != 0) {
        reinterpret_cast<ButtonGroup *>(g_007cd12c)->set(0x3ef, 0);
        char *p = *reinterpret_cast<char **>(g_007d3c40);
        char *v = *reinterpret_cast<char **>(p);
        int adj = *reinterpret_cast<int *>(v + 4);
        reinterpret_cast<VCallShow *>(p + adj)->slot001(0);
        reinterpret_cast<WinHide *>(g_008e9f60)->hide();
    } else {
        reinterpret_cast<ButtonGroup *>(g_007cd12c)->set(0x3ee, 0);
        char *p = *reinterpret_cast<char **>(g_007d3c40);
        char *v = *reinterpret_cast<char **>(p);
        int adj = *reinterpret_cast<int *>(v + 4);
        reinterpret_cast<VCallHide *>(p + adj)->slot002();
        reinterpret_cast<Win *>(g_008e9f60)->show(0);
    }
}
