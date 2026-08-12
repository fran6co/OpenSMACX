// ORIGINAL: 0x00616140 FILE
// name      ?pass_dialog_focus@EditBox@@QAEXXZ
// size      86 bytes
// spans     0x00616140-0x00616196
// prototype void (__thiscall ?pass_dialog_focus@EditBox@@QAEXXZ)(EditBox* this)
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
// address        0x00616140
// measured tier  MISMATCH
// divergence     12
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00616140/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pass_dialog_focus@EditBox@@QAEXXZ  at 0x00616140  (86 bytes)
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

class Win { public:
    void pass_dialog_focus();
};

class EditBox { public:
    void pass_dialog_focus();
};

void EditBox::pass_dialog_focus() {
    char *self = reinterpret_cast<char *>(this);
    char *item = *reinterpret_cast<char **>(self + 0xc4);
    if (item == 0) {
        return;
    }
    if ((*reinterpret_cast<unsigned char *>(self + 0xb3c) & 1) != 0) {
        if (*reinterpret_cast<int *>(item + 0xd8) == *reinterpret_cast<int *>(item + 0xd4) - 1) {
            if (*reinterpret_cast<int *>(item + 0xd4) - 1 >= 0) {
                *reinterpret_cast<int *>(item + 0xd8) = 0;
                *reinterpret_cast<int *>(item + 0xd0) = *reinterpret_cast<int *>(item + 0xcc);
            }
            reinterpret_cast<Win *>(item)->pass_dialog_focus();
            return;
        }
    }
    reinterpret_cast<Win *>(self)->pass_dialog_focus();
}
