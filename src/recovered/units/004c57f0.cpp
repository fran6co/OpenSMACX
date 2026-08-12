// ORIGINAL: 0x004C57F0 BYTE_EXACT FILE
// name      ?release@Midi_Device@@QAEXXZ
// size      51 bytes
// spans     0x004C57F0-0x004C5823
// prototype void (__thiscall ?release@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004C57F0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c57f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?release@Midi_Device@@QAEXXZ  at 0x004C57F0  (51 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 4
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090db40 = (int *)0x0090DB40;

// NOTE: the catalogue's mangled name (?release@Midi_Device@@QAEXXZ) decodes
// the return type as void, so the auto-generated scaffolding declares
// `void Midi_Device::release();` - but the object code ends in
// `xor eax,eax / pop esi / ret`, an explicit zero return that a true void
// leaf would not emit. Verified independently with a same-layout shadow
// class (int-returning): BYTE_EXACT, 51/51. Against the current catalogue
// signature this body is NO_COMPILE (C2556/C2371, return-type redefinition)
// until the catalogue's return type is corrected to int - see PROPOSALS.
class Midi_Device { public:
    int release();
};

int Midi_Device::release() {
    char *self = reinterpret_cast<char *>(this);
    VCall *target = *reinterpret_cast<VCall **>(self + 0x14);
    if (target != 0) {
        target->slot004();
        if (*reinterpret_cast<VCall **>(self + 0x14) != 0) {
            typedef void (__cdecl *ReleaseHook)();
            ReleaseHook hook = *reinterpret_cast<ReleaseHook *>(g_0090db40);
            if (hook != 0) {
                hook();
                *reinterpret_cast<VCall **>(self + 0x14) = 0;
            }
        }
        *reinterpret_cast<VCall **>(self + 0x14) = 0;
    }
    return 0;
}
