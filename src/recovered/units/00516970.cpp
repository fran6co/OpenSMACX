// ORIGINAL: 0x00516970 FILE
// name      ?menu_simple@Console@@QAEXXZ
// size      49 bytes
// spans     0x00516970-0x005169A1
// prototype void (__thiscall ?menu_simple@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00460DD0 0x005BF310
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00516970
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00516970/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?menu_simple@Console@@QAEXXZ  at 0x00516970  (49 bytes)
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
class MainMenu { public:
    void check(int);
};
int __cdecl X_pop(const char *, int (__cdecl *)());

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068a9cc = (int *)0x0068A9CC;
static int *const g_009a6494 = (int *)0x009A6494;

class Console { public:
    void menu_simple();
};

void Console::menu_simple() {
    // NEAR MISS, not byte-exact (MNEMONIC_ONLY): the original loads the
    // field-0x11b0 value into EAX and cleans the X_pop call with
    // `add esp,8`; this toolchain's /O2 register allocator instead puts the
    // value in EDX here (mnemonics match one-for-one, only the register
    // operand differs), and under /O1 it folds the load into `push
    // dword ptr [...]` plus a `pop/pop` cleanup - both worse.
    //
    // RULED OUT: ten source-level variants tried, all producing the exact
    // same EDX choice under /O2 - named vs. inlined field read, swapping
    // statement order, `volatile`, `register`, reusing ONE local for both
    // the flag and the field value (to bias reuse of the just-freed EAX),
    // a comma-expression folding the RMW and the read into one argument
    // expression, and routing the read through a forced-inline getter (to
    // invoke the "return value lives in eax" convention). Also confirmed in
    // ISOLATION, outside this function: a bare N-argument forwarding probe
    // (unrelated to this call) shows this same EAX-after-just-freed-EAX
    // reuse never happens for a fresh single-use temporary under these
    // project flags - see 0x00624D00's near miss for the same family of
    // issue (fixed register-count/window choices by the scheduler, not
    // reachable from portable source).
    MainMenu *menu = reinterpret_cast<MainMenu *>(reinterpret_cast<char *>(this) + 0x22a2c);
    *g_009a6494 &= 0xffdfffff;
    int vehicle = *reinterpret_cast<int *>(reinterpret_cast<char *>(menu) + 0x11b0);
    menu->check(vehicle);
    X_pop(reinterpret_cast<const char *>(g_0068a9cc), 0);
}
