// ORIGINAL: 0x0048C060 BYTE_EXACT FILE
// name      ?parse_it@@YAXPADPBD@Z
// size      57 bytes
// spans     0x0048C060-0x0048C099
// prototype void (__cdecl ?parse_it@@YAXPADPBD@Z)(int8* srcFileID, int8* sectionID)
// callers   5   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BECA0 0x005FD570 0x00625880
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0048C060
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0048c060/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?parse_it@@YAXPADPBD@Z  at 0x0048C060  (57 bytes)
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

// NOTE: the stale opaque struct placeholders this scaffold shipped with
// (int8/sectionID/srcFileID) did not match the mangled types - X_text_open
// is `PAD` (char*) then `PBD` (const char*), not two identical opaque
// pointers. The live scaffolding (emit_translation_unit + declfix) derives
// these correctly from the catalogue; spelled out plainly here instead.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl X_text_open(char *a1, const char *a2);
int __cdecl parse_string(char *input, char *output);
char* __cdecl text_get();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b7d00 = (int *)0x009B7D00;
static int *const g_009b86a0 = (int *)0x009B86A0;

void __cdecl parse_it(char *a1, const char *a2) {
    *reinterpret_cast<char *>(g_009b86a0) = 0;
    if (X_text_open(a1, a2) == 0) {
        text_get();
        parse_string(*reinterpret_cast<char **>(g_009b7d00),
                     reinterpret_cast<char *>(g_009b86a0));
    }
}
