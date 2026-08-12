// ORIGINAL: 0x00611910 FILE
// name      sub_611910
// size      76 bytes
// spans     0x00611910-0x0061191E;0x006117E0-0x0061181E
// prototype 
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00608E10 0x00610120 0x0064557F
// PRESERVED UNIT - measured SHARED_TAIL.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00611910
// measured tier  SHARED_TAIL
// refusal        1 span(s) are COMDAT-folded and claimed by another function; no per-function verdict is well defined
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00611910/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_611910  at 0x00611910  (76 bytes)
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
class Dialog { public:
    ~Dialog();
};
class GraphicWin { public:
    ~GraphicWin();
};
class SpriteBox { public:
    ~SpriteBox();
};
int __cdecl fn_0064557f();

// Adjustor thunk: `this -= *(int*)(this-4); this -= 0xa18;` then a tail
// JMP (not CALL) into the scalar deleting destructor at 0x6117E0, which
// itself runs SpriteBox::~SpriteBox(), Dialog::~Dialog() (at `this+0xa18`,
// the same 0xa18 this thunk subtracts - so Dialog is the second base, right
// after a GraphicWin-sized first base) and GraphicWin::~GraphicWin(), then
// optionally `operator delete` on a low bit of the stack flag.
//
// Confirmed WALL, not a body defect: probing this address with the
// verifier reports span_classes.shared=1, tier SHARED_TAIL,
// "1 span(s) are COMDAT-folded and claimed by another function; no
// per-function verdict is well defined." The bytes at 0x6117E0 are /Gy
// output the linker folded into (and attributed to) a DIFFERENT symbol; one
// translation unit cannot recreate that sharing, so no source form here can
// ever score BYTE_EXACT or even MISMATCH - it is REFUSED before comparison.
// UNREACHABLE, matching the COMDAT-glue class of wall already documented for
// this project. Left as the closest semantic reconstruction reached.
int __stdcall sub_611910(int a1) {
    char *self = reinterpret_cast<char *>(a1);
    int vbase_disp = *reinterpret_cast<int *>(self - 4);
    char *target = self - vbase_disp - 0xa18;
    return reinterpret_cast<int>(target);
}
