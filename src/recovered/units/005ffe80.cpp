// ORIGINAL: 0x005FFE80 BYTE_EXACT FILE
// name      sub_5ffe80
// size      61 bytes
// spans     0x005FFE80-0x005FFEBD
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005FFE80
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ffe80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ffe80  at 0x005FFE80  (61 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006692b0 = (int *)0x006692B0;
static int *const g_009b8194 = (int *)0x009B8194;

typedef void *HWND;
typedef unsigned int UINT;
typedef unsigned int WPARAM;
typedef long LPARAM;
typedef long LRESULT;

// g_006692b0 above is the IAT slot for DefWindowProcA, but a plain
// function-pointer load/call through it emits a direct `call rel32`
// instead of the original's indirect `call dword ptr [slot]` - a Win32
// import must be declared `dllimport` for VC6 to emit the indirect form.
extern "C" __declspec(dllimport) LRESULT __stdcall DefWindowProcA(HWND, UINT, WPARAM, LPARAM);

// param_1 (a1) is read nowhere in the body - only a2..a5 are touched.
// The message id is a `guard ? obj : 0` shape (neg/sbb/and, per the
// lever this pass keeps re-finding); msg is 0x203 (WM_LBUTTONDBLCLK)
// when a2 is set, else 0x201 (WM_LBUTTONDOWN); lparam packs a3/a4 as
// the usual (x, y) 16-bit pair.
int __cdecl sub_5ffe80(int a1, int a2, int a3, int a4, int a5) {
    unsigned short hi = static_cast<unsigned short>(a4);
    unsigned short lo = static_cast<unsigned short>(a3);
    LPARAM lparam = (static_cast<LPARAM>(hi) << 0x10) | lo;
    WPARAM wparam = a5;
    UINT msg = (a2 ? 2 : 0) + 0x201;
    return DefWindowProcA(reinterpret_cast<HWND>(*g_009b8194), msg, wparam, lparam);
}
