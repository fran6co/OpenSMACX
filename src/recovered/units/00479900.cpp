// ORIGINAL: 0x00479900 BYTE_EXACT FILE
// name      ?UNK2@MultiWin@@QAEXXZ
// size      100 bytes
// spans     0x00479900-0x00479964
// prototype void (__thiscall ?UNK2@MultiWin@@QAEXXZ)(MultiWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00479900
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00479900/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK2@MultiWin@@QAEXXZ  at 0x00479900  (100 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939ed8 = (int *)0x00939ED8;
static int *const g_0093f660 = (int *)0x0093F660;

class MultiWin { public:
    void UNK2();
};

void MultiWin::UNK2() {
    char *self = reinterpret_cast<char *>(this);
    int v1158 = *reinterpret_cast<int *>(self + 0x1158);
    int v11b4 = *reinterpret_cast<int *>(self + 0x11b4);
    int v1100 = *reinterpret_cast<int *>(self + 0x1100);
    int total = v1158 * 7 + v1100 + v11b4;
    if (*g_0093f660 != 0) {
        int v112c = *reinterpret_cast<int *>(self + 0x112c);
        total += v112c + v11b4 + 0xe;
    }
    *reinterpret_cast<int *>(self + 0xa38) = 0;
    *reinterpret_cast<int *>(self + 0xa3c) = 0;
    *reinterpret_cast<int *>(self + 0xa40) = 0x8a;
    *reinterpret_cast<int *>(self + 0xa44) = total;
    *g_00939ed8 |= 0x40;
}
