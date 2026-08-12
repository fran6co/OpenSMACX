// ORIGINAL: 0x005F50E0 BYTE_EXACT FILE
// name      ?remove_parent@Win@@QAAXPAUWin@@@Z
// size      86 bytes
// spans     0x005F50E0-0x005F5136
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005F50E0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f50e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?remove_parent@Win@@QAAXPAUWin@@@Z  at 0x005F50E0  (86 bytes)
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

struct Win;

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

// This table's address is walked/indexed, not just loaded, so it is an
// extern array rather than a folded literal (the address itself does work).
extern Win *g_009b6e48[];
static int *const g_009b7b34 = (int *)0x009B7B34;

struct Win {
    void __cdecl remove_parent(Win*);
};

void __cdecl Win::remove_parent(Win* a1) {
    if (this == 0) {
        return;
    }
    int count = *g_009b7b34;
    for (int i = 0; i < count; i++) {
        if (g_009b6e48[i] == this) {
            if (i < count) {
                count--;
                *g_009b7b34 = count;
                reinterpret_cast<VCall *>(this)->slot002();
                count = *g_009b7b34;
                if (i < count) {
                    Win **p = &g_009b6e48[i];
                    int n = count - i;
                    do {
                        *p = p[1];
                        p++;
                    } while (--n);
                }
            }
            return;
        }
    }
}
