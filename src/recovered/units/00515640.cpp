// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00515640
// name           ?toggle_flatten@Console@@QAEXXZ
// size           81 bytes
// measured tier  NO_COMPILE
// refusal        u00515640.cpp(55) : error C2065: 'BOOL' : undeclared identifier u00515640.cpp(55) : error C2146: syntax error : missing ')' before identifier 'saveFactions' u00
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00515640/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?toggle_flatten@Console@@QAEXXZ  at 0x00515640  (81 bytes)
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

typedef int BOOL;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Menu { public:
    int check_menu_item(int, int);
    int uncheck_menu_item(int, int);
};
void __cdecl draw_map(int);
void __cdecl prefs_save(BOOL saveFactions);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0094b468 = (int *)0x0094B468;
static int *const g_009a6494 = (int *)0x009A6494;

class Console { public:
    void toggle_flatten();
};

void Console::toggle_flatten() {
    int flags = *g_009a6494 ^ 0x10000;
    *g_009a6494 = flags;
    if (flags & 0x10000) {
        reinterpret_cast<Menu *>(reinterpret_cast<char *>(this) + 0x22a2c)->check_menu_item(2, 0x209);
    } else {
        reinterpret_cast<Menu *>(reinterpret_cast<char *>(this) + 0x22a2c)->uncheck_menu_item(2, 0x209);
    }
    *g_0094b468 = *g_009a6494;
    prefs_save(0);
    draw_map(1);
}
