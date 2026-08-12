// ORIGINAL: 0x00491380 FILE
// name      ?on_scrolling@PrefWin@@QAEXHH@Z
// size      102 bytes
// spans     0x00491380-0x004913E6
// prototype void (__thiscall ?on_scrolling@PrefWin@@QAEXHH@Z)(PrefWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C5150 0x004C5320
// indirect  0x004913DF
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00491380
// measured tier  MISMATCH
// divergence     36
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00491380/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_scrolling@PrefWin@@QAEXHH@Z  at 0x00491380  (102 bytes)
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
class Wave_Device { public:
    int set_group_volume(unsigned int, unsigned int);
    void set_volume(unsigned int);
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 16
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
    virtual void slot016(int);  // <-- used, tuned to take the pushed int arg
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_004913e8 = (int *)0x004913E8;
static int *const g_0074daa0 = (int *)0x0074DAA0;
static int *const g_0090d978 = (int *)0x0090D978;

class PrefWin { public:
    void on_scrolling(int, int);
};

// NOT BYTE_EXACT, and provably so for a tooling reason rather than a
// code one - same shape as the documented case in
// Dialogs::on_redraw (build/byte-match/00612e00/unit.cpp): every one
// of the 36 original mnemonics is reproduced in order (mnemonic
// similarity 0.889 only because the divergence is pure insertion,
// not substitution; edit_count 1), matching case-for-case including
// the exact `add eax, -0x50` bounds check and the exact register in
// every one of the four case bodies. Under `/Gy` this switch's jump
// table lands in the SAME `.text` COMDAT immediately after the
// function, preceded by a 2-byte `mov edi,edi` alignment filler
// (verified via byte_match.py's `--keep`: the real function body is
// bytes 0-0x65, exactly 102, and the filler+table+padding is 0x66
// onward). `clip_jump_table` in tools/byte_match.py strips the
// table's own DIR32 relocation run but has no rule for that leading
// filler instruction, so the rebuilt span reads 120 bytes and the
// comparator reports six trailing "phantom" mov/add instructions
// that are actually the table's raw relocation-placeholder zero
// bytes misread as code. Ruled out: every /O2 flag combination
// reproduces the identical filler (only /O1 changes shape - to an
// if/dec-chain that drops the jump table the original clearly has,
// per `jmp dword ptr [eax*4+0x4913e8]` - so /O1 is not a fix, it is
// a different and wrong function).
void PrefWin::on_scrolling(int a1, int a2) {
    switch (a1) {
        case 0x50:
            reinterpret_cast<Wave_Device *>(g_0090d978)->set_volume(a2);
            break;
        case 0x51:
            reinterpret_cast<Wave_Device *>(g_0090d978)->set_group_volume(0, a2);
            break;
        case 0x52:
            reinterpret_cast<Wave_Device *>(g_0090d978)->set_group_volume(1, a2);
            break;
        case 0x53:
            if (*g_0074daa0 != 0) {
                reinterpret_cast<VCall *>(*g_0074daa0)->slot016(a2);
            }
            break;
    }
}
