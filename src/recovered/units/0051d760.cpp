// ORIGINAL: 0x0051D760 FILE
// name      sub_51d760
// size      85 bytes
// spans     0x0051D760-0x0051D7B5
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0051D760
// measured tier  MISMATCH
// divergence     8
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0051d760/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_51d760  at 0x0051D760  (85 bytes)
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
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007af59c = (int *)0x007AF59C;
static int *const g_00939e58 = (int *)0x00939E58;
static int *const g_00939e5c = (int *)0x00939E5C;
static int *const g_00939ed8 = (int *)0x00939ED8;
static int *const g_00939ee8 = (int *)0x00939EE8;
static int *const g_00939eec = (int *)0x00939EEC;
static int *const g_00939ef0 = (int *)0x00939EF0;
static int *const g_00939ef4 = (int *)0x00939EF4;
static int *const g_009b7b1c = (int *)0x009B7B1C;
static int *const g_009b7b20 = (int *)0x009B7B20;

class Sub51D760Owner { public:
    void sub_51d760();
};

void Sub51D760Owner::sub_51d760() {
    int a = *g_009b7b1c;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bcc) = a;
    *g_00939e5c = a;
    int b = *g_009b7b20 - *g_007af59c;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bd0) = b;
    *g_00939e58 = b;
    *g_00939ef4 = b;
    *g_00939ee8 = 0;
    *g_00939eec = 0;
    *g_00939ef0 = *g_00939e5c;
    *g_00939ed8 |= 1;
}
