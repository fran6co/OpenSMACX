// ORIGINAL: 0x004C3A70 FILE
// name      ?UNK1@WorldWin@@QAEXXZ
// size      57 bytes
// spans     0x004C3A70-0x004C3AA9
// prototype void (__thiscall ?UNK1@WorldWin@@QAEXXZ)(WorldWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004C3A70
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c3a70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK1@WorldWin@@QAEXXZ  at 0x004C3A70  (57 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007af51c = (int *)0x007AF51C;
static int *const g_007af520 = (int *)0x007AF520;
static int *const g_007af524 = (int *)0x007AF524;
static int *const g_007af528 = (int *)0x007AF528;
static int *const g_00939ed8 = (int *)0x00939ED8;
static int *const g_00939f08 = (int *)0x00939F08;
static int *const g_00939f0c = (int *)0x00939F0C;
static int *const g_00939f10 = (int *)0x00939F10;
static int *const g_00939f14 = (int *)0x00939F14;

class WorldWin { public:
    void UNK1();
};

// NEAR MISS, not BYTE_EXACT - see build notes / agent report for
// 0x004C3A70. Aggregate-initialising the first three source words hoists
// them into eax/ecx/edx exactly as the original does (a 3-loose-locals
// declaration puts the wrong value in each register every time this was
// tried; the struct-initializer form is the one that landed a/b/c on the
// right registers). What remains unmatched is pure instruction
// SCHEDULING: the original interposes the 0x7AF528 load between the
// 0x939F08 and 0x939F0C stores, and the 0x939F10 store between the OR and
// the 0x939ED8 store, and no permutation of these four trailing
// statements reproduces that exact interleaving (24 permutations tried,
// mnemonic-for-mnemonic identical, closest is 17 bytes off in constants
// -- actually shape-identical, only 2 instruction-pair orderings swapped).
struct Temp3 { int x, y, z; };

void WorldWin::UNK1() {
    Temp3 t = { *g_007af51c, *g_007af520, *g_007af524 };
    *g_00939f08 = t.x;
    *g_00939f0c = t.y;
    *g_00939f14 = *g_007af528;
    *g_00939ed8 = *g_00939ed8 | 8;
    *g_00939f10 = t.z;
}
