// ORIGINAL: 0x005DDDA0 FILE
// name      ?wrap_height_flying@Buffer@@QAEHPADH@Z
// size      50 bytes
// spans     0x005DDDA0-0x005DDDD2
// prototype int (__thiscall ?wrap_height_flying@Buffer@@QAEHPADH@Z)(Buffer* this, int8*, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DDDE0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005DDDA0
// measured tier  MISMATCH
// divergence     9
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ddda0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wrap_height_flying@Buffer@@QAEHPADH@Z  at 0x005DDDA0  (50 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b3a94 = (int *)0x009B3A94;
static int *const g_009b3a98 = (int *)0x009B3A98;
static int *const g_009b3a9c = (int *)0x009B3A9C;

class Buffer { public:
    int wrap_height_flying(char *);
    int wrap_height_flying(char *, int);
};

int Buffer::wrap_height_flying(char *a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    register char *text = a1;
    int value = *reinterpret_cast<int *>(self + 0x10);
    *reinterpret_cast<int *>(self + 0x18) = 0;
    *reinterpret_cast<int *>(self + 0x14) = value;
    *reinterpret_cast<int *>(self + 0x51C) = 0;
    *g_009b3a94 = 0;
    *g_009b3a9c = 0;
    *g_009b3a98 = a2;
    return wrap_height_flying(text);
}
