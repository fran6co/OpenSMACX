// ORIGINAL: 0x00611A90 BYTE_EXACT FILE
// name      ?close@EditGroup@@QAEXXZ
// size      96 bytes
// spans     0x00611A90-0x00611AF0
// prototype void (__thiscall ?close@EditGroup@@QAEXXZ)(EditGroup* this)
// callers   9   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00608F50 0x00644EF2
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00611A90
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00611a90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close@EditGroup@@QAEXXZ  at 0x00611A90  (96 bytes)
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
class Dialog { public:
    void close();
};
int __cdecl _free();
extern "C" void __cdecl free(void *ptr);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0
class VCall { public:
    virtual void slot000(int);  // <-- used, tuned: original pushes one arg
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b8fcc = (int *)0x009B8FCC;

class EditGroup { public:
    void close();
};

void EditGroup::close() {
    char *self = reinterpret_cast<char *>(this);
    char *esi = self + 4;
    int ebx = 10;
    do {
        int *slot_a = reinterpret_cast<int *>(esi);
        int ctrl = *slot_a;
        if (ctrl != 0) {
            reinterpret_cast<VCall *>(ctrl)->slot000(1);
            *slot_a = 0;
            *reinterpret_cast<int *>(esi + 0x50) = 10;
        }
        int *slot_b = reinterpret_cast<int *>(esi + 0x28);
        int freeable = *slot_b;
        if (freeable != 0) {
            free(reinterpret_cast<void *>(freeable));
            *slot_b = 0;
        }
        esi += 4;
    } while (--ebx != 0);

    *reinterpret_cast<int *>(self + 0x84) = *g_009b8fcc;

    char *vtable = *reinterpret_cast<char **>(self);
    int voffset = *reinterpret_cast<int *>(vtable + 8);
    reinterpret_cast<Dialog *>(self + voffset)->close();
}
