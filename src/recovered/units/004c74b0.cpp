// ORIGINAL: 0x004C74B0 FILE
// name      sub_4c74b0
// size      64 bytes
// spans     0x004C74B0-0x004C74F0
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006453E0 0x00645460 0x0064557F 0x0064558A
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004C74B0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c74b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c74b0  at 0x004C74B0  (64 bytes)
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
extern "C" int _strcpy();  // arity unknown
int __cdecl _strlen();
int __cdecl fn_0064557f();
int __cdecl fn_0064558a();

// `ecx` is read with no stack access before the stack parameter is ever
// touched, and `ret 4` pops exactly one stack slot: this is a __thiscall
// member taking the one explicit `char *` argument, not a free `__stdcall`
// function.
//
// `strlen`/`strcpy` (the declfix-respelled names) are VC6 intrinsics: naming
// them that way gets the call INLINED into a `repne scasb`/`rep movsd`
// expansion the original does not have here. Spelling them `_strlen`/
// `_strcpy` (a plain, non-"recognised" identifier - precedent in
// src/recovered/005bf350.cpp) keeps them real calls.
//
// Binding `this`/`name` to a named local flips the esi/edi assignment
// (both ways tried: named locals for both, named locals for just one,
// `register` hints) relative to the original's esi=this/edi=name pairing.
// Only inlining every access as a fresh `reinterpret_cast<char*>(this)`
// expression - no local at all - reproduces it.
int __cdecl _strlen(const char *);
char *__cdecl _strcpy(char *, const char *);
void __cdecl fn_0064557f(void *);
void *__cdecl fn_0064558a(unsigned int);

class Unknown_4c74b0 { public:
    void set_name(char *name);
};

void Unknown_4c74b0::set_name(char *name) {
    if (*reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x200) != 0) {
        fn_0064557f(*reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x200));
    }
    void *newbuf = fn_0064558a(_strlen(name) + 1);
    *reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x200) = newbuf;
    _strcpy(reinterpret_cast<char *>(newbuf), name);
}
