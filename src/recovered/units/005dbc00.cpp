// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005DBC00
// name           ?wrap_cent@Buffer@@QAEHPAVFont@@PADPAURECT@@@Z
// size           79 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005dbc00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wrap_cent@Buffer@@QAEHPAVFont@@PADPAURECT@@@Z  at 0x005DBC00  (79 bytes)
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

struct Font;
struct RECT;
struct int8;

// ---- callees, declared and never defined (a definition would be inlined) ----

class Buffer { public:
    int wrap_cent(Font*, int8*, RECT*);
};

class BufferWrapCent2 { public:
    int wrap_cent(int8*, RECT*);
};

int Buffer::wrap_cent(Font* a1, int8* a2, RECT* a3) {
    char *self = reinterpret_cast<char *>(this);
    Font **p0 = reinterpret_cast<Font **>(self + 0x52c);
    Font *save0 = *p0;
    int save1 = *reinterpret_cast<int *>(self + 0x530);
    int save2 = *reinterpret_cast<int *>(self + 0x534);
    if (a1 != 0 && *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 8) != 0) {
        *p0 = a1;
        *reinterpret_cast<int *>(self + 0x530) = 0;
        *reinterpret_cast<int *>(self + 0x534) = 0;
    }
    int result = reinterpret_cast<BufferWrapCent2 *>(this)->wrap_cent(a2, a3);
    *p0 = save0;
    *reinterpret_cast<int *>(self + 0x530) = save1;
    *reinterpret_cast<int *>(self + 0x534) = save2;
    return result;
}
