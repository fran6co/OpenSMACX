// ORIGINAL: 0x00634ED0 ?set_def_name@FileBox@@QAEHPAD@Z 0x00634ED0-0x00634F1E FILE BYTE_EXACT
// size      78 bytes
// prototype int (__thiscall ?set_def_name@FileBox@@QAEHPAD@Z)(FileBox* this, int8*)
// callers   3   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645470
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00634ED0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00634ed0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_def_name@FileBox@@QAEHPAD@Z  at 0x00634ED0  (78 bytes)
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

struct int8;

// The staged `_strcat`/`_strlen` decls are stale (arity-less, and would
// pick up a second leading underscore from extern "C" __cdecl decoration).
// Correct extern "C" declarations, unused otherwise, cost nothing.
extern "C" unsigned int strlen(const char *);
extern "C" char *strcat(char *, const char *);
// VC6 treats strlen/strcat as intrinsics under /O2; the original calls
// out to the real CRT routines, so intrinsic substitution has to be
// disabled for this translation unit.
#pragma function(strlen, strcat)

class FileBox { public:
    int set_def_name(int8*);
};

// `field_104_` (a char[] starting at offset 0x104) is the default-name
// buffer: a1==0 clears its first byte; otherwise strlen(a1) is bounds
// checked against 0x104 before the buffer is zeroed and `a1` is
// concatenated onto it (an idiom equivalent to strcpy, written with the
// CRT's strcat instead - matches the disassembly's callee exactly).
int FileBox::set_def_name(int8* a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0) {
        self[0x104] = 0;
        return 0;
    }
    if (strlen(reinterpret_cast<const char *>(a1)) > 0x104) {
        return 3;
    }
    char *buf = self + 0x104;
    *buf = 0;
    strcat(buf, reinterpret_cast<const char *>(a1));
    return 0;
}
