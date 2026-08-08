// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00459500
// name           ??0InfoWin@@QAE@XZ
// size           118 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00459500/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0InfoWin@@QAE@XZ  at 0x00459500  (118 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
// Destructors added: the two unwind funclets at 0x655260/0x65526b tear down
// Time_/Font_ if a LATER member's constructor throws, and VC6 only emits
// that partial-construction protection when it can see there IS something
// to destroy - a class with a declared ctor but no dtor gets none (measured:
// probe with 3 ctor-only members compiles the plain 3-call body, no SEH
// frame at all; adding `~Type();` to each reproduces the frame exactly).
class Font { public:
    Font();
    ~Font();
};
class PushButton { public:
    PushButton();
    ~PushButton();
};
class Time { public:
    Time();
    ~Time();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00655276 = (int *)0x00655276;
static int *const g_006754b8 = (int *)0x006754B8;

// Real members, spaced with raw padding to land Time_/Font_/PushButton_ at
// the offsets the disassembly reads (0x30, 0x58, 0x9d0): no vtable pointer
// is ever stored in this constructor, so nothing else here is virtual, and
// giving these THREE members real (non-trivial) constructors is what makes
// the compiler emit the SEH partial-construction unwind frame on its own -
// placement-new into an opaque empty class does not.
class InfoWin { public:
    InfoWin();
private:
    char pad0_[0x30];
    Time time_;
    char pad1_[0x58 - 0x30 - sizeof(Time)];
    Font font_;
    char pad2_[0x9d0 - 0x58 - sizeof(Font)];
    PushButton pushbutton_;
};

InfoWin::InfoWin() {
}
