// ORIGINAL: 0x00517180 FILE
// name      ?show_version@Console@@QAEXXZ
// size      65 bytes
// spans     0x00517180-0x005171C1
// prototype void (__thiscall ?show_version@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048C0A0 0x00625EC0
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00517180
// measured tier  NO_COMPILE
// refusal        u00517180.cpp(44) : error C2061: syntax error : identifier 'int8' u00517180.cpp(45) : error C2065: 'int8' : undeclared identifier u00517180.cpp(45) : error C205
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00517180/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?show_version@Console@@QAEXXZ  at 0x00517180  (65 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl parse_says(int nID, int8* input, int gender, int pluralality);
int __cdecl popp(int8*, int8*, int, int8*, int (__cdecl *)());

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068aa7c = (int *)0x0068AA7C;
static int *const g_0068aa8c = (int *)0x0068AA8C;
static int *const g_00691870 = (int *)0x00691870;
static int *const g_00691874 = (int *)0x00691874;
static int *const g_00691b0c = (int *)0x00691B0C;

class Console { public:
    void show_version();
};

void Console::show_version() {
    parse_says(0, reinterpret_cast<int8 *>(*g_00691870), -1, -1);
    parse_says(1, reinterpret_cast<int8 *>(*g_00691874), -1, -1);
    popp(reinterpret_cast<int8 *>(*g_00691b0c), reinterpret_cast<int8 *>(g_0068aa8c), 0, reinterpret_cast<int8 *>(g_0068aa7c), 0);
}
