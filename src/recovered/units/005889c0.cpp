// ORIGINAL: 0x005889C0 BYTE_EXACT FILE
// name      ?popups_medium@@YAXXZ
// size      106 bytes
// spans     0x005889C0-0x00588A2A
// prototype 
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006048C0 0x006049C0 0x00609D20
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005889C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005889c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?popups_medium@@YAXXZ  at 0x005889C0  (106 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
// The on-disk prototypes carried a leading BasePop*/Dialog* parameter that
// the live catalogue no longer has: `?set_def_string_font@BasePop@@QAAHP
// AUFont@@PAUFont@@PAUFont@@PAUFont@@@Z` mangles to four Font* parameters
// and nothing else - these are static members, called with no `this` setup
// in the disassembly (no `mov ecx` precedes any of the three calls).
struct Font;
int __cdecl fn_006048c0(Font*, Font*, Font*, Font*);
int __cdecl fn_006049c0(Font*, Font*, Font*);
int __cdecl fn_00609d20(Font*, Font*, Font*);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00696ed0 = (int *)0x00696ED0;
static int *const g_00696ed4 = (int *)0x00696ED4;
static int *const g_006a721c = (int *)0x006A721C;
static int *const g_0094576c = (int *)0x0094576C;
static int *const g_00945808 = (int *)0x00945808;
static int *const g_0094580c = (int *)0x0094580C;

void __cdecl popups_medium() {
    Font *f1 = *reinterpret_cast<Font **>(g_00945808);
    Font *f2 = *reinterpret_cast<Font **>(g_0094580c);
    Font *f3 = *reinterpret_cast<Font **>(g_0094576c);
    *g_006a721c = 3;
    fn_006048c0(f3, f2, f1, 0);

    f1 = *reinterpret_cast<Font **>(g_00945808);
    f2 = *reinterpret_cast<Font **>(g_0094580c);
    f3 = *reinterpret_cast<Font **>(g_0094576c);
    fn_00609d20(f3, f2, f1);

    f1 = *reinterpret_cast<Font **>(g_00945808);
    f2 = *reinterpret_cast<Font **>(g_0094580c);
    f3 = *reinterpret_cast<Font **>(g_0094576c);
    fn_006049c0(f3, f2, f1);

    *g_00696ed0 = 2;
    *g_00696ed4 = 2;
}
