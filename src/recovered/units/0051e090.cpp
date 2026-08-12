// ORIGINAL: 0x0051E090 BYTE_EXACT FILE
// name      ?set_tamper@@YAXXZ
// size      114 bytes
// spans     0x0051E090-0x0051E102
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0051E0B4 0x0051E0D8 0x0051E0F1 0x0051E0F7
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0051E090
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0051e090/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_tamper@@YAXXZ  at 0x0051E090  (114 bytes)
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

// ---- callees ----
// `call dword ptr [0x669004/8/10]` are IAT-indirect calls to advapi32
// registry functions - `dllimport` reproduces the shape without modelling
// the slots as data (see 0x00592E70's `timeGetTime`, the same pattern).
typedef unsigned long DWORD;
typedef long LONG;
typedef void *HKEY;
typedef HKEY *PHKEY;
typedef unsigned char BYTE;
typedef DWORD *LPDWORD;
typedef const char *LPCSTR;
typedef void *LPSECURITY_ATTRIBUTES;

extern "C" __declspec(dllimport) LONG __stdcall RegCreateKeyExA(
    HKEY, LPCSTR, DWORD, char *, DWORD, DWORD, LPSECURITY_ATTRIBUTES,
    PHKEY, LPDWORD);
extern "C" __declspec(dllimport) LONG __stdcall RegSetValueExA(
    HKEY, LPCSTR, DWORD, DWORD, const BYTE *, DWORD);
extern "C" __declspec(dllimport) LONG __stdcall RegCloseKey(HKEY);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static char *const g_0068add8 = (char *)0x0068ADD8;
static char *const g_0068ae20 = (char *)0x0068AE20;
static char *const g_0068ae24 = (char *)0x0068AE24;
static BYTE *const g_0093a060 = (BYTE *)0x0093A060;
static BYTE *const g_0093a9f8 = (BYTE *)0x0093A9F8;
static char *const g_0093aa00 = (char *)0x0093AA00;

void __cdecl set_tamper() {
    HKEY key;
    LONG status = RegCreateKeyExA(
        reinterpret_cast<HKEY>(0x80000002), g_0068add8, 0, g_0093aa00, 0,
        0xf003f, 0, &key, 0);
    if (status == 0) {
        RegSetValueExA(key, g_0068ae20, 0, 3, g_0093a9f8, 4);
        RegSetValueExA(key, g_0068ae24, 0, 3, g_0093a060, 0x4b0);
        RegCloseKey(key);
    }
}
