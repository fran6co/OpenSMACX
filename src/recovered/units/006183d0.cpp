// ORIGINAL: 0x006183D0 ?set_buffer@Caviar@@QAEHPAUBuffer@@@Z 0x006183D0-0x00618426 FILE
// size      86 bytes
// prototype int (__thiscall ?set_buffer@Caviar@@QAEHPAUBuffer@@@Z)(Caviar* this, Buffer*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x006183D0
// measured tier  MISMATCH
// divergence     17
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006183d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_buffer@Caviar@@QAEHPAUBuffer@@@Z  at 0x006183D0  (86 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
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

struct Buffer;

class Caviar { public:
    int set_buffer(Buffer*);
};

int Caviar::set_buffer(Buffer* a1) {
    char *self = reinterpret_cast<char *>(this);
    if ((*reinterpret_cast<unsigned char *>(self + 4) & 1) == 0) {
        return 7;
    }
    if (a1 == 0) {
        return 0x10;
    }
    *reinterpret_cast<int *>(self + 0xed) = 0;
    *reinterpret_cast<int *>(self + 0xf1) = 0;
    *reinterpret_cast<Buffer **>(self + 0x14) = a1;
    *reinterpret_cast<int *>(self + 0xf5) = 0xff;
    *reinterpret_cast<int *>(self + 0xf9) = 0xff;
    *reinterpret_cast<int *>(self + 0xfd) = 0x100;
    *reinterpret_cast<int *>(self + 0x101) = 0x100;
    return 0;
}
