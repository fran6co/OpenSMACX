// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x006019F0
// name           ?button@BasePop@@QAEHPAD@Z
// size           75 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006019f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?button@BasePop@@QAEHPAD@Z  at 0x006019F0  (75 bytes)
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
class StringStruct { public:
    int add(int);
};

class BasePop { public:
    int button(int8*);
};

// member | BasePop | 0x2150 | embedded StringStruct (add(int) at 0x401100) |
//        | lea ecx,[esi+0x2150] followed by a direct (non-virtual) call to
//        | ?add@StringStruct@@QAEHH@Z with ecx as `this`
// member | BasePop | 0x216c | StringStruct field: char* (relative +0x1c
//        | from the embedded StringStruct at 0x2150) | `mov [ecx+0x1c],eax`
//        | stores a1, the subject's own char* parameter
// member | BasePop | 0x20f4 | int counter, incremented on successful add |
//        | `inc dword ptr [esi+0x20f4]` on the add()==0 path only
int BasePop::button(int8* a1) {
    if (a1 == 0) {
        return 3;
    }
    char *self = reinterpret_cast<char *>(this);
    StringStruct *ss = reinterpret_cast<StringStruct *>(self + 0x2150);
    *reinterpret_cast<int8 **>(reinterpret_cast<char *>(ss) + 0x1c) = a1;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(ss) + 0x20) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(ss) + 0x24) = 0;
    if (ss->add(0) != 0) {
        return 1;
    }
    ++*reinterpret_cast<int *>(self + 0x20f4);
    return 0;
}
