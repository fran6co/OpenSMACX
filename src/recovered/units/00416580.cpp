// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00416580
// name           ?bubble_me_elmo2@BaseWin@@QAGXPADHH@Z
// size           66 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00416580/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?bubble_me_elmo2@BaseWin@@QAGXPADHH@Z  at 0x00416580  (66 bytes)
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

// The stored scaffold does not parse: `struct this;` names a reserved
// keyword (C2143), `int8` is an incomplete type used where the mangled
// name's `PAD` plainly means `char *`, and the definition head treats `this`
// as an explicit parameter while ALSO keeping the class receiver, double
// counting it. Rebuilt from the mangled name and the disassembly instead.
struct RECT {
    long left, top, right, bottom;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class Win { public:
    void client_to_screen(int*, int*);
    void set_bubble_text(char*, RECT*);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007ae820 = (int *)0x007AE820;

// `ret 0xc` pops exactly the 3 explicit arguments - no receiver's worth of
// extra 4 bytes, and the body never reads `this` (ecx is overwritten with
// the fixed 0x7AE820 singleton before either call). A real `__stdcall`
// member instead passes `this` as an implicit FIRST stack slot (`ret 0x10`,
// every offset shifted by 4) - confirmed by compiling it that way first.
// Written as a plain free function instead; the comparison is over the
// object's code, not a symbol lookup.
void __stdcall bubble_me_elmo2(char *a1, int a2, int a3) {
    Win *win = reinterpret_cast<Win *>(g_007ae820);
    win->client_to_screen(&a2, &a3);
    RECT rect;
    rect.left = a2;
    rect.top = a3;
    rect.right = a2;
    rect.bottom = a3;
    win->set_bubble_text(a1, &rect);
}
