// ORIGINAL: 0x00618E30 FILE
// name      ?vx_read@Caviar@@SAXHPADJ@Z
// CORRECTED from ?vx_read@Caviar@@QAAXHPADJ@Z
//   SHAPE_EXACT with all three parameter reads off by exactly +4
//   (0x4/0x8/0xc against 0x8/0xc/0x10) - the width of the `this` that
//   QAA inserts and the bytes do not have
// size      24 bytes
// spans     0x00618E30-0x00618E48
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0064A178
// PRESERVED UNIT - measured SHAPE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00618E30
// measured tier  SHAPE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00618e30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?vx_read@Caviar@@QAAXHPADJ@Z  at 0x00618E30  (24 bytes)
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
extern "C" int __cdecl __read(int, char *, unsigned int);

class Caviar { public:
    void __cdecl vx_read(int a1, char *a2, long a3);
};

void __cdecl Caviar::vx_read(int a1, char *a2, long a3) {
    __read(a1, a2, a3);
}
