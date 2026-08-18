// ORIGINAL: 0x004C3990 ?set_world_map@WorldWin@@QAEXXZ 0x004C3990-0x004C39CB FILE BYTE_EXACT
// size      59 bytes
// prototype void (__thiscall ?set_world_map@WorldWin@@QAEXXZ)(WorldWin* this)
// callers   4   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED9D0 0x0062B870
// indirect  0x004C39BB
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004C3990
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c3990/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_world_map@WorldWin@@QAEXXZ  at 0x004C3990  (59 bytes)
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
// This body dispatches through slot(s): 2
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007cd12c = (int *)0x007CD12C;
static int *const g_007d3c40 = (int *)0x007D3C40;
static int *const g_008e9f60 = (int *)0x008E9F60;

class WorldWin { public:
    void set_world_map();
};

// This call goes through a virtual base, not the plain VCall shim: the
// original's `mov/mov/lea/mov/call` sequence at the divergence point IS the
// compiler's own vbtable-adjustment thunk, reusing the pre-adjustment
// registers for the post-adjustment vtable load rather than reloading via
// the freshly computed pointer. A hand pointer-arithmetic reproduction of
// that expression emits `add`, not `lea`, and is two bytes short; declaring
// the real virtual-inheritance relationship lets the compiler regenerate
// the same thunk byte-for-byte.
class VBaseIface {
public:
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
};
class Adjustor : public virtual VBaseIface {
};

void WorldWin::set_world_map() {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0x239cc) = 0;
    reinterpret_cast<ButtonGroup *>(g_007cd12c)->set(0x3ee, 0);
    reinterpret_cast<Adjustor *>(*g_007d3c40)->slot2();
    reinterpret_cast<Win *>(g_008e9f60)->show(0);
}
