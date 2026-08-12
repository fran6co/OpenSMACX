// ORIGINAL: 0x0053E470 BYTE_EXACT FILE
// name      ?diplomacy_ends@@YAXHH@Z
// size      76 bytes
// spans     0x0053E470-0x0053E4BC
// prototype void (__cdecl ?diplomacy_ends@@YAXHH@Z)(int factionID, int)
// callers   3   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0043F5C0 0x00530600 0x00593450
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0053E470
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0053e470/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?diplomacy_ends@@YAXHH@Z  at 0x0053E470  (76 bytes)
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

struct factionID;

// ---- callees, declared and never defined (a definition would be inlined) ----
class DiploPop { public:
    void stop();
};
class NetDaemon { public:
    void close_channel();
};
void __cdecl synch_diplo(int factionID, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068d43c = (int *)0x0068D43C;
static int *const g_0068d440 = (int *)0x0068D440;
static int *const g_00733990 = (int *)0x00733990;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093fab4 = (int *)0x0093FAB4;
static int *const g_0093fab8 = (int *)0x0093FAB8;
static int *const g_0093fac8 = (int *)0x0093FAC8;
static int *const g_0093facc = (int *)0x0093FACC;

void __cdecl diplomacy_ends(int a1, int a2) {
    *g_0093fab4 = 0;
    *g_0093fab8 = 0;
    *g_0093fac8 = 0;
    *g_0093facc = 0;
    reinterpret_cast<DiploPop *>(g_00733990)->stop();
    synch_diplo(a1, a2);
    reinterpret_cast<NetDaemon *>(g_0093cd90)->close_channel();
    *g_0068d440 = -1;
    *g_0068d43c = -1;
}
