// ORIGINAL: 0x0062E250 BYTE_EXACT FILE
// name      ?poll_services@Net@@QAEHPAUServiceList@@@Z
// size      73 bytes
// spans     0x0062E250-0x0062E299
// prototype int (__thiscall ?poll_services@Net@@QAEHPAUServiceList@@@Z)(Net* this, ServiceList*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0065020E
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0062E250
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0062e250/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?poll_services@Net@@QAEHPAUServiceList@@@Z  at 0x0062E250  (73 bytes)
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

struct ServiceList;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int DirectPlayEnumerateA();  // arity unknown

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00630bf0 = (int *)0x00630BF0;
static int *const g_009be5f0 = (int *)0x009BE5F0;

class Net { public:
    int poll_services(ServiceList*);
};

int Net::poll_services(ServiceList* a1) {
    if (!a1) {
        return 0x10;
    }
    char *self = reinterpret_cast<char *>(this);
    if (!*reinterpret_cast<int *>(self + 0x6e8)) {
        return 7;
    }
    // The emitted extern for DirectPlayEnumerateA is `()` (unknown arity),
    // which cannot be called with args at all. The absence of any stack
    // cleanup after `call 0x65020e` (vs. a caller-cleanup `add esp,8`)
    // shows it is really __stdcall(void*, void*) - a cast on the address
    // of the (wrongly nullary) declared symbol keeps a direct relative
    // call instead of redeclaring the extern with a conflicting signature.
    *g_009be5f0 = reinterpret_cast<int>(a1);
    typedef int(__stdcall * EnumFn)(void *, void *);
    int result = (reinterpret_cast<EnumFn>(&DirectPlayEnumerateA))(
        reinterpret_cast<void *>(g_00630bf0), 0);
    *g_009be5f0 = 0;
    return result != 0;
}
