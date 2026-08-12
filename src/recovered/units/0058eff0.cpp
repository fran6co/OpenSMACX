// ORIGINAL: 0x0058EFF0 BYTE_EXACT FILE
// name      ?desktop_close@@YAXXZ
// size      65 bytes
// spans     0x0058EFF0-0x0058F031
// prototype 
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00408710 0x0043C1A0 0x004710E0 0x004B9F80 0x005D4E40
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0058EFF0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0058eff0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?desktop_close@@YAXXZ  at 0x0058EFF0  (65 bytes)
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
class BaseWin { public:
    void close();
};
class DesignWin { public:
    void close();
};
class GraphicWin { public:
    void close();
};
class StatusWin { public:
    void close();
};
void __cdecl mapwin_system_shutdown();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_0071f2b0 = (int *)0x0071F2B0;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007fd648 = (int *)0x007FD648;
static int *const g_008c5568 = (int *)0x008C5568;
static int *const g_008e9f60 = (int *)0x008E9F60;

// `?hide@Win@@QAEXXZ` is reached by a tail JMP, not a `call`, so it never
// lands in this address's callee list (emit_translation_unit's callgraph
// only tracks `call` targets) and the live scaffold's `Win` (pulled in only
// for its field layout, from a different function's structural promotion)
// carries no methods. A same-shaped shim class stands in for it.
class WinHideShim { public:
    void hide();
};

void __cdecl desktop_close() {
    reinterpret_cast<DesignWin *>(g_0071f2b0)->close();
    reinterpret_cast<BaseWin *>(g_006a7628)->close();
    mapwin_system_shutdown();
    reinterpret_cast<StatusWin *>(g_008c5568)->close();
    reinterpret_cast<GraphicWin *>(g_008e9f60)->close();
    reinterpret_cast<GraphicWin *>(g_007fd648)->close();
    reinterpret_cast<WinHideShim *>(g_007ae820)->hide();
}
