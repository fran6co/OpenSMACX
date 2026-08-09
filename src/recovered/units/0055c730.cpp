// ORIGINAL: 0x0055C730 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0055C730
// name           ?pact_attacks@@YAXHH@Z
// size           259 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0055c730/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pact_attacks@@YAXHH@Z  at 0x0055C730  (259 bytes)
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
class NetMsg { public:
    void pop(const char*, int, int, const char*);
};
int __cdecl parse_says(int nID, char* input, int gender, int pluralality);
char* __cdecl get_he_she(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068e5d4 = (int *)0x0068E5D4;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

void __cdecl pact_attacks(int a1, int a2) {
    int off = a1 * 0x59c;
    *g_009bbff0 = 0;
    *g_009bbfec = *reinterpret_cast<int *>(0x946a50 + off);
    parse_says(0, reinterpret_cast<char *>(0x946a9c + off), -1, -1);

    *g_009bbfec = *reinterpret_cast<int *>(0x946a50 + off);
    *g_009bbff0 = 0;
    parse_says(1, reinterpret_cast<char *>(0x946a84 + off), -1, -1);

    *g_009bbfec = *reinterpret_cast<int *>(0x946d4c + off);
    *g_009bbff0 = *reinterpret_cast<int *>(0x946d50 + off);
    parse_says(2, reinterpret_cast<char *>(0x946d34 + off), -1, -1);

    parse_says(3, get_he_she(a1, 0), -1, -1);

    int off2 = a2 * 0x59c;
    *g_009bbfec = *reinterpret_cast<int *>(0x946d4c + off2);
    *g_009bbff0 = *reinterpret_cast<int *>(0x946d50 + off2);
    parse_says(4, reinterpret_cast<char *>(0x946d34 + off2), -1, -1);

    reinterpret_cast<NetMsg *>(g_00805338)->pop(
        reinterpret_cast<const char *>(g_0068e5d4), 0x1388, 0, 0);
}
