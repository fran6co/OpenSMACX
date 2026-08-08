// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0044CB80
// name           ?help_create_link@@YAXPAD00@Z
// size           66 bytes
// measured tier  NO_COMPILE
// refusal        u0044cb80.cpp(65) : error C2664: 'my_strcat' : cannot convert parameter 1 from 'struct int8 *' to 'char *' Types pointed to are unrelated; conversion requires r
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0044cb80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?help_create_link@@YAXPAD00@Z  at 0x0044CB80  (66 bytes)
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

struct int8;

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl _strcat();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068353c = (int *)0x0068353C;
static int *const g_00683540 = (int *)0x00683540;
static int *const g_00683544 = (int *)0x00683544;

// The given `extern "C" char *strcat(char*, const char*);` is the CRT's own
// name, and VC6 recognises that exact spelling as a string intrinsic under
// /O2: it expands the call into an inline strlen(scasb)+rep-movs sequence
// instead of a real `call`, which the original does not have. Declaring the
// same signature under an unrecognised name forces a genuine call - the
// relocation target is masked by the comparison, so the name does not need
// to match.
extern "C" char *my_strcat(char *, const char *);

void __cdecl help_create_link(int8* a1, int8* a2, int8* a3) {
    my_strcat(a1, reinterpret_cast<char *>(g_00683544));
    my_strcat(a1, a2);
    my_strcat(a1, reinterpret_cast<char *>(g_00683540));
    my_strcat(a1, a3);
    my_strcat(a1, reinterpret_cast<char *>(g_0068353c));
}
