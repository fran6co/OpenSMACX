// ORIGINAL: 0x005CE450 BYTE_EXACT FILE
// name      sub_5ce450
// size      83 bytes
// spans     0x005CE450-0x005CE4A3
// prototype 
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005CE472 0x005CE496
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005CE450
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ce450/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ce450  at 0x005CE450  (83 bytes)
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
static int *const g_00800000 = (int *)0x00800000;

// `ecx` is read with no stack access at entry (`mov esi, ecx`), so the
// receiver is a real `this`: this is a `__thiscall` member with zero
// explicit parameters (bare `ret`, nothing popped beyond `this`).
//
// The indirect call is NOT a true C++ virtual dispatch: at the `call`
// instant `ecx` holds `*p1` (the callee's own "vtable" cell), not `p1`
// itself, and no `add esp,4` follows the call even though an argument was
// pushed - so the callee, not the caller, cleans the stack. A real
// __thiscall virtual call needs `this` live in ecx at the call and a
// 0-arg member leaves nothing to push. Both point at a plain `__stdcall`
// function pointer stored in the callee's own header, taking the callee
// itself as its one explicit argument - so the VCall shim above is not
// used here; a typedef says the same thing without pretending it is a
// polymorphic call.
typedef void (__stdcall *ReleaseFn)(void *);

class Sub5ce450 { public:
    int call();
};

int Sub5ce450::call() {
    char *self = reinterpret_cast<char *>(this);
    char *target = *reinterpret_cast<char **>(self);
    if (*reinterpret_cast<unsigned int *>(target + 4) & 0x800000) {
        if (*reinterpret_cast<int *>(self + 0x3c8) != 0) {
            char *p1 = *reinterpret_cast<char **>(target + 0x58);
            if (p1 != 0) {
                ReleaseFn fn = *reinterpret_cast<ReleaseFn *>(*reinterpret_cast<char **>(p1) + 8);
                fn(p1);
                char *target2 = *reinterpret_cast<char **>(self);
                *reinterpret_cast<char **>(target2 + 0x58) = 0;
            }
        }
        if (*reinterpret_cast<int *>(self + 0x3cc) != 0) {
            char *target3 = *reinterpret_cast<char **>(self);
            char **slot = reinterpret_cast<char **>(target3 + 0x54);
            char *p2 = *slot;
            if (p2 != 0) {
                ReleaseFn fn = *reinterpret_cast<ReleaseFn *>(*reinterpret_cast<char **>(p2) + 8);
                fn(p2);
                *slot = 0;
            }
        }
    }
    return 0;
}
