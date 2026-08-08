// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0060CF10
// name           sub_60cf10
// size           61 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0060cf10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_60cf10  at 0x0060CF10  (61 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 9
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();  // <-- used
};

// UNREACHABLE - matches the measured "/Gy COMDAT GLUE" wall, one step
// removed. The catalogued 61-byte span is TWO pieces:
//
//   0x0060CF10  sub ecx, [ecx - 4]        (3 bytes)
//   0x0060CF13  jmp 0x0060CF20            (5 bytes, E9 rel32)
//   0x0060CF18  int3 x8                   (padding up to a 16-byte boundary)
//   0x0060CF20  mov eax, [ecx - 0x48]     the REAL, self-sufficient body,
//               ... ret 4 / ret 4         two exits, both `ret 4`
//
// `sub ecx, [ecx - 4]` reads its adjustment from MEMORY, not a compile-time
// constant - unlike 0x00421830's `lea esi, [ecx - 0x21A6C]` and the
// thunk1/thunk2_MapWin adjustors in src/adjustor_thunks.cpp, which all bake
// in a literal displacement. A runtime-read vtordisp is the compiler's own
// synthesized adjustor for a virtual override reached through a virtual
// base, emitted automatically by the FRONT END - it is not spelled in any
// C++ source, and there is no source-level lever that asks the compiler to
// re-derive it. Confirmed empirically: translating the reachable half (from
// 0x0060CF20 on, matching the Ghidra hypothesis's field reads and the slot-9
// virtual dispatch through the generated VCall shim) as an ordinary
// `__thiscall` member compiles straight-line with no jump and no padding -
// `sub` vs `mov` at instruction #0, 46 bytes against 61. One translation
// unit's ordinary output cannot reproduce a jmp-then-16-byte-aligned-pad
// boundary; that is linker/front-end layout, not expressible source. Left
// UNREACHABLE rather than forced.
int __stdcall sub_60cf10(int a1) {
    return a1;  // PLACEHOLDER - see UNREACHABLE note above
}
