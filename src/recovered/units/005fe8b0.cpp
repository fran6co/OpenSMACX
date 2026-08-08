// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005FE8B0
// name           ?start_cycle@Palette@@QAEHH@Z
// size           75 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005fe8b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?start_cycle@Palette@@QAEHH@Z  at 0x005FE8B0  (75 bytes)
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
class Time { public:
    int start();
};

class Palette { public:
    int start_cycle(int);
};

// MISMATCH, closest reached (72/75 bytes, mnemonic similarity 0.8364).
// The array walked at `this+0x404` is 5 entries of 0x10 bytes: an `int id`
// at +0x0 and a `Time *` at +0x4 (0x408 total), proven by the post-loop
// `mov ecx,[eax+0x404]` / success-path `mov ecx,[eax+0x408]` pair and the
// direct (non-virtual) call to `Time::start`.
//
// RULED OUT: matching the two-register prologue (`push esi; push edi`).
// The original keeps `a1` resident in EDI and a walking pointer in ESI for
// the whole function, popping BOTH before the branch on success/failure so
// both tails share one epilogue - the post-loop recheck then recomputes the
// found entry's address from `this`/`i` (`shl eax,4; add eax,ecx`) because
// ESI is already gone by then. Every C shape tried here (array indexing
// throughout; a raw walking pointer reused for the recheck; a raw walking
// pointer plus a separately-preserved base pointer; recomputing the base
// fresh from `this` at the recheck) has the compiler keep only ONE register
// resident (never both ESI-class and EDI-class survivors at once), so `a1`
// keeps getting reloaded from the stack instead of pinned in a register.
// Closest: plain array indexing (`entries[i]`) end to end, given below.
struct CycleEntry {
    int id;
    Time *time;
    int pad_8_;
    int pad_c_;
};

int Palette::start_cycle(int a1) {
    char *self = reinterpret_cast<char *>(this);
    CycleEntry *entries = reinterpret_cast<CycleEntry *>(self + 0x404);
    int i = 0;
    do {
        if (entries[i].id == a1 || entries[i].id == -1) {
            break;
        }
        ++i;
    } while (i < 5);
    if (entries[i].id != a1) {
        return 0xb;
    }
    entries[i].time->start();
    return 0;
}
