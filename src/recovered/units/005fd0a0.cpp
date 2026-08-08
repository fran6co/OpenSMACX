// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005FD0A0
// name           ?do_all_mouse@@YAXXZ
// size           117 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005fd0a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?do_all_mouse@@YAXXZ  at 0x005FD0A0  (117 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl check_net();
void __cdecl do_net();
void __cdecl do_video();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669340 = (int *)0x00669340;
static int *const g_00669358 = (int *)0x00669358;
static int *const g_0066935c = (int *)0x0066935C;
static int *const g_009b7b9c = (int *)0x009B7B9C;

typedef int (__stdcall *PeekMessageFn)(void *, void *, unsigned int, unsigned int, unsigned int);
typedef int (__stdcall *TranslateMessageFn)(void *);
typedef int (__stdcall *DispatchMessageFn)(void *);

// Opaque MSG buffer: 0x1c bytes, matching the stack allocation and never
// touched field-by-field, only ever passed by address.
struct Msg28 {
    uint8_t data_[0x1C];
};

void __cdecl do_all_mouse() {
    Msg28 msg;
    *g_009b7b9c = 8;
    TranslateMessageFn translate = *reinterpret_cast<TranslateMessageFn *>(g_0066935c);
    PeekMessageFn peek = *reinterpret_cast<PeekMessageFn *>(g_00669358);
    DispatchMessageFn dispatch = *reinterpret_cast<DispatchMessageFn *>(g_00669340);
    int got;
    do {
        do_video();
        check_net();
        got = peek(&msg, 0, 0x200, 0x209, 1);
        if (got) {
            translate(&msg);
            dispatch(&msg);
            *g_009b7b9c = 8;
        }
    } while (got);
    *g_009b7b9c = 0;
    do_net();
}
