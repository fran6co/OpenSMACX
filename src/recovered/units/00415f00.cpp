// ORIGINAL: 0x00415F00 BYTE_EXACT FILE
// name      ?gov_activate@BaseWin@@QAEXXZ
// size      112 bytes
// spans     0x00415F00-0x00415F70
// prototype void (__thiscall ?gov_activate@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004E5FE0 0x004EC3B0 0x004F5F30
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00415F00
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00415f00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?gov_activate@BaseWin@@QAEXXZ  at 0x00415F00  (112 bytes)
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
int __cdecl base_compute(int);
void __cdecl base_doctors();
void __cdecl base_reset(int baseID, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00689370 = (int *)0x00689370;
static int *const g_0090ea30 = (int *)0x0090EA30;

class BaseWin { public:
    void gov_activate();
};

void BaseWin::gov_activate() {
    char *base = reinterpret_cast<char *>(*g_0090ea30);
    if (*reinterpret_cast<unsigned int *>(base + 0x38) & 0x80000000) {
        if (*reinterpret_cast<unsigned char *>(base + 0x38) & 1) {
            *reinterpret_cast<unsigned int *>(base + 0x30) &= 0x7fffffff;
            base = reinterpret_cast<char *>(*g_0090ea30);
            *reinterpret_cast<int *>(base + 0x4c) = 0;
            base_reset(*g_00689370, 1);
            base = reinterpret_cast<char *>(*g_0090ea30);
        }
        if (*reinterpret_cast<unsigned char *>(base + 0x38) & 0x40) {
            *reinterpret_cast<int *>(base + 0x78) = 0;
            base = reinterpret_cast<char *>(*g_0090ea30);
            *reinterpret_cast<int *>(base + 0x7c) = 0;
            base = reinterpret_cast<char *>(*g_0090ea30);
            *reinterpret_cast<int *>(base + 0x80) = 0;
            base_compute(1);
            base_doctors();
        }
    }
}
