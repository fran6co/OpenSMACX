// ORIGINAL: 0x00629110 FILE
// name      ??0StringBox@@QAE@XZ
// size      266 bytes
// spans     0x00629110-0x00629208;0x00663300-0x00663312
// prototype void (__thiscall ??0StringBox@@QAE@XZ)(StringBox* this)
// callers   6   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4CF0 0x006051D0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00629110
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00629110/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0StringBox@@QAE@XZ  at 0x00629110  (266 bytes)
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
class GraphicWin { public:
    GraphicWin();
};
class Scroll { public:
    Scroll();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00663308 = (int *)0x00663308;
static int *const g_006693a0 = (int *)0x006693A0;
static int *const g_006693a4 = (int *)0x006693A4;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006698c0 = (int *)0x006698C0;
static int *const g_006698c4 = (int *)0x006698C4;
static int *const g_0066adc0 = (int *)0x0066ADC0;
static int *const g_0066adc8 = (int *)0x0066ADC8;
static int *const g_0066b0ec = (int *)0x0066B0EC;
static int *const g_0067ff88 = (int *)0x0067FF88;
static int *const g_009b3374 = (int *)0x009B3374;

class StringBox { public:
    StringBox();
};

// Placement-new, declared locally rather than pulling in <new>: the
// scaffolding's VC6 dialect note says nothing about it either way, and this
// keeps the unit self-contained.
inline void *operator new(unsigned int, void *place) { return place; }

StringBox::StringBox() {
    char *self = reinterpret_cast<char *>(this);
    new (self) GraphicWin();
    new (self + 0xa20) Scroll();

    *reinterpret_cast<void **>(self + 0x2b74) = g_0066b0ec;
    *reinterpret_cast<void **>(self + 0x2b98) = g_006693ac;
    int saved = *g_009b3374;
    *reinterpret_cast<int *>(self + 0x2b9c) = saved;
    *g_009b3374 = 0;

    char *p1 = *reinterpret_cast<char **>(self + 0x2b74);
    *reinterpret_cast<void **>(self + 0x2b70) = g_006693a4;
    int disp1 = *reinterpret_cast<int *>(p1 + 4);
    *reinterpret_cast<void **>(self + 0x2b74 + disp1) = g_006693a0;

    char *p2 = *reinterpret_cast<char **>(self + 0x2b74);
    *reinterpret_cast<int *>(self + 0x2b78) = 0;
    *reinterpret_cast<int *>(self + 0x2b7c) = 0;
    *reinterpret_cast<int *>(self + 0x2b80) = 0;
    *reinterpret_cast<int *>(self + 0x2b84) = 0;
    *reinterpret_cast<int *>(self + 0x2b88) = 0;
    *reinterpret_cast<void **>(self + 0x2b70) = g_006698c4;
    int disp2 = *reinterpret_cast<int *>(p2 + 4);
    *reinterpret_cast<void **>(self + 0x2b74 + disp2) = g_006698c0;

    *reinterpret_cast<int *>(self + 0xa1c) = 0;
    *reinterpret_cast<int *>(self + 0x2b6c) = 0;
    *reinterpret_cast<int *>(self + 0xa14) = 0;
    *reinterpret_cast<void **>(self) = g_0066adc8;
    *reinterpret_cast<void **>(self + 0x444) = g_0066adc0;
    *reinterpret_cast<int *>(self + 0xa18) = -1;
}
