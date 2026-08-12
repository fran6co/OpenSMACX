// ORIGINAL: 0x00559290 FILE
// name      ??0FontQueue@@QAE@XZ
// size      61 bytes
// spans     0x00559290-0x005592CD
// prototype void (__thiscall ??0FontQueue@@QAE@XZ)(FontQueue* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006457C2
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00559290
// measured tier  NO_COMPILE
// refusal        u00559290.cpp(44) : error C4234: nonstandard extension used : '__thiscall' keyword reserved for future use u00559290.cpp(44) : error C4234: nonstandard extensio
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00559290/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0FontQueue@@QAE@XZ  at 0x00559290  (61 bytes)
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
void __stdcall fn_006457c2(void*, unsigned int size, int count, void (__thiscall *)(void*), void (__thiscall *)(void*));

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00618ea0 = (int *)0x00618EA0;
static int *const g_00618ee0 = (int *)0x00618EE0;

class FontQueue { public:
    FontQueue();
};

struct FQCtorShim { void m(); };
typedef void (FQCtorShim::*FQCtorPtr)();

union FQCtorCast {
    void *addr;
    FQCtorPtr method;
};

FontQueue::FontQueue() {
    FQCtorCast ctor, dtor;
    ctor.addr = g_00618ea0;
    dtor.addr = g_00618ee0;
    fn_006457c2(this, 0x28, 3, ctor.method, dtor.method);
    char *base = reinterpret_cast<char *>(this) + 0x84;
    for (int i = 0; i < 3; i++) {
        *reinterpret_cast<int *>(base - 0xc) = -999;
        *reinterpret_cast<int *>(base) = 0;
        *reinterpret_cast<int *>(base + 0xc) = i;
        base += 4;
    }
}
