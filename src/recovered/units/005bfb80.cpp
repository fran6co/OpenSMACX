// ORIGINAL: 0x005BFB80 BYTE_EXACT FILE
// name      ?X_pop_ask@@YAHPBDPADP6AHXZH@Z
// size      64 bytes
// spans     0x005BFB80-0x005BFBC0
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BFBC0 0x006453E0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005BFB80
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005bfb80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?X_pop_ask@@YAHPBDPADP6AHXZH@Z  at 0x005BFB80  (64 bytes)
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

// The scaffold's own guesses do not parse: `struct __cdecl;` names a
// reserved calling-convention keyword as an identifier (C2059), `struct
// int8;` is an incomplete type used by value, and
// `int (__cdecl *)() a3` misplaces the declarator. Redeclared below with
// the head the mangled name actually spells - `PBD`/`PAD` are
// `const char *`/`char *`, not an opaque `int8`.
//
// `strlen` (the plain spelling) is a compiler intrinsic here too, expanded
// inline as `repne scasb` regardless of whether the string is a compile-time
// constant - confirmed empirically, the same class of substitution as
// `memcpy`. Named `_strlen` to route around it, same as the memcpy fix.
extern "C" unsigned int __cdecl _strlen(const char *s);

// The second `X_pop_ask` overload this one forwards to -
// `?X_pop_ask@@YAHPADPBDHPADP6AHXZH@Z` - takes a different parameter list
// (mangled `PAD PBD H PAD P6AHXZ H`), which is a distinct C++ overload, not
// a redeclaration of this function.
int __cdecl X_pop_ask(char *p1, const char *p2, int p3, char *p4, int (__cdecl *p5)(), int p6);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b8aa8 = (int *)0x009B8AA8;

int __cdecl X_pop_ask(const char *a1, char *a2, int (__cdecl *a3)(), int a4) {
    if (a2 == 0) {
        return -1;
    }
    return X_pop_ask(reinterpret_cast<char *>(g_009b8aa8), a1, _strlen(a2), a2, a3, a4);
}
