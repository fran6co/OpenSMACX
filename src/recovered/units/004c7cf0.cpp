// ORIGINAL: 0x004C7CF0 ?set_fname@Midi@@QAEHPBD@Z 0x004C7CF0-0x004C7D42 FILE BYTE_EXACT
// symbol    ?set_fname@Midi@@QAEHPAC@Z
// size      82 bytes
// prototype int (__thiscall ?set_fname@Midi@@QAEHPBD@Z)(Midi* this, int8*)
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006453E0 0x00645460 0x0064557F 0x0064558A
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004C7CF0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c7cf0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_fname@Midi@@QAEHPBD@Z  at 0x004C7CF0  (82 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int _strcpy();  // arity unknown
int __cdecl _strlen();
int __cdecl fn_0064557f();
int __cdecl fn_0064558a();

class Midi { public:
    int set_fname(int8*);
};

// The staged preamble's `_strcpy`/`_strlen` decls are stale (arity-less,
// and `_strlen`/`_strcpy` as C++ names each pick up a SECOND leading
// underscore from the `extern "C"` __cdecl decoration, landing on
// `__strlen`/`__strcpy` - not the CRT's real `_strlen`/`_strcpy` symbol).
// Correct extern "C" declarations, left unused, cost nothing.
extern "C" unsigned int strlen(const char *);
extern "C" char *strcpy(char *, const char *);
// VC6 recognises strlen/strcpy as intrinsics under /O2 and inlines them
// (rep scasb / rep movsb) unless told not to; the original calls out to
// the real CRT routines, so the intrinsic substitution has to be turned
// off for this translation unit.
#pragma function(strlen, strcpy)

int Midi::set_fname(int8* a1) {
    if (a1) {
        void *old = *reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x4c);
        if (old != 0) {
            operator delete(old);
        }
        *reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x4c) = 0;
        unsigned int len = strlen(reinterpret_cast<const char *>(a1));
        void *buf = operator new(len + 1);
        *reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x4c) = buf;
        strcpy(reinterpret_cast<char *>(buf), reinterpret_cast<const char *>(a1));
        return 0;
    }
    return 10;
}
