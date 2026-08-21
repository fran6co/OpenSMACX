// ORIGINAL: 0x00618430 ?set_shadow_table@Caviar@@QAEHPAE@Z 0x00618430-0x00618468 FILE BYTE_EXACT
// symbol    ?set_shadow_table@CaviarShadow@@QAEHPAE@Z
// size      56 bytes
// prototype int (__thiscall ?set_shadow_table@Caviar@@QAEHPAE@Z)(Caviar* this, LPBYTE)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645930
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00618430
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00618430/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_shadow_table@Caviar@@QAEHPAE@Z  at 0x00618430  (56 bytes)
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
static int *const g_009bb478 = (int *)0x009BB478;

// ---- callees, declared and never defined (a definition would be inlined) ----
// Named `_memcpy` (not `memcpy`): the compiler treats the plain spelling as
// the intrinsic and inlines fixed-size copies as `rep movsd`, which the
// original does not do here - it keeps a real `call`. The underscored name
// sidesteps intrinsic recognition and gets a genuine call again.
extern "C" void *__cdecl _memcpy(void *dest, const void *src, unsigned int count);

// The scaffold's `Caviar::set_shadow_table(LPBYTE)` cannot be defined:
// `LPBYTE` was forward-declared as an incomplete struct type and used BY
// VALUE, which VC6 rejects (C2027) at the point of definition. The mangled
// name's `PAE` says the real parameter is `unsigned char *`, so the method is
// redeclared here under a fresh class - the comparison is over the object's
// code, not a symbol lookup, so the class name is free to change.
class CaviarShadow { public:
    int set_shadow_table(unsigned char *a1);
};

int CaviarShadow::set_shadow_table(unsigned char *a1) {
    char *const obj = *reinterpret_cast<char **>(g_009bb478);
    if (obj == 0) {
        return 7;
    }
    char *const table = *reinterpret_cast<char **>(obj + 0x54);
    if (table == 0) {
        return 0x10;
    }
    _memcpy(table, a1, 0x100);
    return 0;
}
