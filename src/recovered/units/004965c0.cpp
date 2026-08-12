// ORIGINAL: 0x004965C0 FILE
// name      ?on_key_click@QuayleWin@@QAEHHH@Z
// size      76 bytes
// spans     0x004965C0-0x0049660C
// prototype int (__thiscall ?on_key_click@QuayleWin@@QAEHHH@Z)(QuayleWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00609F20
// indirect  0x004965D8 0x004965F6
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004965C0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004965c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_key_click@QuayleWin@@QAEHHH@Z  at 0x004965C0  (76 bytes)
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
class GraphicWin { public:
    void close();
};
class ListBox { public:
    void close();
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 58, 62
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
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();  // <-- used
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

class QuayleWin { public:
    int on_key_click(int, int);
};

// field_2144_ (offset 0x2144) is a one-shot flag like MonuWin's at
// 0x4769B0: not -1 means "close in progress", clear it and dispatch
// slot062; already -1 means finish closing the embedded ListBox (at
// 0x2158) and the window itself. a1/a2 (the clicked key) are unused.
//
// MNEMONIC_ONLY, not BYTE_EXACT: same length (76B) and mnemonic count
// (21) as the original; the sole remaining divergence is the SAME
// scheduling swap seen at 0x4769B0 - the original issues the vtable-ptr
// load (`mov edx,[esi]`) BEFORE the independent flag store
// (`mov [esi+0x2144],-1`), the rebuilt issues the store first. Writing
// the comparison as `*flag + 1 != 0` (rather than `!= -1`) already
// recovered the `inc eax; je` encoding the original uses for the
// guard - a real, source-visible fix - but this second reordering is
// the backend's scheduler on two dependency-free ops, not source form.
int QuayleWin::on_key_click(int a1, int a2) {
    int *flag = reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x2144);
    if (*flag + 1 != 0) {
        *flag = -1;
        reinterpret_cast<VCall *>(this)->slot062();
        return 1;
    }
    reinterpret_cast<ListBox *>(reinterpret_cast<char *>(this) + 0x2158)->close();
    reinterpret_cast<VCall *>(this)->slot058();
    reinterpret_cast<GraphicWin *>(this)->close();
    return 1;
}
