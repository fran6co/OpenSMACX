// ORIGINAL: 0x00616030 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00616030
// name           ?text_position@EditBox@@QAEXH@Z
// size           261 bytes
// measured tier  MISMATCH
// divergence     67
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00616030/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?text_position@EditBox@@QAEXH@Z  at 0x00616030  (261 bytes)
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
class Buffer { public:
    int text_width(char*, int);
};
// `_strcat`/`_strlen` are the real CRT functions; the stale nullary
// scaffolding here could not be called with the arguments the call sites
// push. VC6's /O2 substitutes an inline expansion for both when it
// recognises the name, so the intrinsic form is turned off explicitly -
// the original calls the real functions (0x645470, 0x6453e0).
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);
#pragma function(strcat, strlen)

class EditBox { public:
    int text_position(int);
};

int EditBox::text_position(int a1) {
    // Reach fields by offset - the class is deliberately empty.
    char *self = reinterpret_cast<char *>(this);
    if (a1 < *reinterpret_cast<int *>(self + 0xb34)) {
        return *reinterpret_cast<int *>(self + 0xb18);
    }

    a1 -= *reinterpret_cast<int *>(self + 0xb34);
    char local[256];
    char *text;

    if ((*reinterpret_cast<unsigned char *>(self + 0xb3c) & 2) != 0) {
        local[0] = 0;
        strcat(local, self + 0xa14);
        char *p = local;
        if (*p != 0) {
            char c;
            do {
                *p = '*';
                c = p[1];
                ++p;
            } while (c != 0);
        }
        text = local;
    } else {
        text = self + 0xa14 + *reinterpret_cast<int *>(self + 0xb18);
    }

    if (*text == 0) {
        return 0;
    }

    int length = static_cast<int>(strlen(text));
    int count = 1;
    if (length > 1) {
        Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);
        do {
            if (buffer->text_width(text, count) > a1) {
                break;
            }
            ++count;
        } while (count < length);
    }

    if (count < *reinterpret_cast<int *>(self + 0xb44)) {
        return *reinterpret_cast<int *>(self + 0xb18) + count - 1;
    }
    return *reinterpret_cast<int *>(self + 0xb18) + count;
}
