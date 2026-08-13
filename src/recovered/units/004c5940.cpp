// ORIGINAL: 0x004C5940 FILE
// name      ??0Wave_In_Device@@QAE@XZ
// size      60 bytes
// spans     0x004C5940-0x004C597C
// prototype void (__thiscall ??0Wave_In_Device@@QAE@XZ)(Wave_In_Device* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006465F0
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004C5940
// measured tier  NO_COMPILE
// refusal        u004c5940.cpp(63) : error C2065: 'memset' : undeclared identifier
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c5940/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0Wave_In_Device@@QAE@XZ  at 0x004C5940  (60 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl _memset();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066e098 = (int *)0x0066E098;
static int *const g_0066e1f0 = (int *)0x0066E1F0;

class Wave_In_Device { public:
    Wave_In_Device();
};

Wave_In_Device::Wave_In_Device() {
    // The base class's vtable is set (0x66e098), its member at +4 zeroed via
    // an explicit memset(4), then the derived fields are cleared and this
    // class's own vtable (0x66e1f0) is stamped over the base's last - the
    // classic MSVC base-then-derived vtable-store sequence.
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<void **>(self) = g_0066e098;
    memset(self + 4, 0, 4);
    *reinterpret_cast<int *>(self + 0xc) = 0;
    *reinterpret_cast<int *>(self + 0x18) = 0;
    *reinterpret_cast<int *>(self + 0x1c) = 0;
    *reinterpret_cast<int *>(self + 0x10) = 0;
    *reinterpret_cast<int *>(self + 0x14) = 0;
    *reinterpret_cast<int *>(self + 8) = 0x7f;
    *reinterpret_cast<void **>(self) = g_0066e1f0;
}
