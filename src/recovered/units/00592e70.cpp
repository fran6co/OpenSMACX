// ORIGINAL: 0x00592E70 FILE
// name      ?message_base@@YAXHHPADH@Z
// size      106 bytes
// spans     0x00592E70-0x00592EDA
// prototype 
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532940 0x00645460
// indirect  0x00592E87
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00592E70
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00592e70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?message_base@@YAXHHPADH@Z  at 0x00592E70  (106 bytes)
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
class NetDaemon { public:
    void send_message(char*, unsigned int, int);
};
// `_strcpy` is the CRT's strcpy(char*, const char*) -> char*; the earlier
// nullary `int _strcpy()` scaffold could not even be called with the
// arguments the call site pushes.
extern "C" char *strcpy(char *, const char *);
// VC6's /O2 substitutes an inline rep-movs expansion for a handful of CRT
// functions including strcpy; the original calls the real function, so the
// intrinsic form has to be turned off for it explicitly.
#pragma function(strcpy)
// `call dword ptr [0x669368]` is the IAT-indirect shape MSVC always emits
// for a call to a DLL-imported function - `dllimport` reproduces it without
// needing to model the slot as a global.
extern "C" __declspec(dllimport) unsigned long __stdcall timeGetTime();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8bc = (int *)0x0093E8BC;
// 0x93f664 is incremented before the call and decremented after it (a
// reentrancy guard). The decrement compiles to a bare `dec dword ptr
// [addr]` in the original - a read-modify-write ON the address itself,
// which the fixed-address-pointer spelling folds into a load/modify/store
// under /O2 (measured on other addresses). `extern int` keeps it.
extern int g_0093f664;

void __cdecl message_base(int a1, int a2, char* a3, int a4) {
    // Reach fields by offset - the class is deliberately empty.
    // A 0x2c-byte message assembled on the stack and handed to
    // NetDaemon::send_message, matching the `push 0x2c` length constant.
    struct Message {
        short id;
        int global_1;
        int time;
        int global_2;
        int a2;
        char text[24];
    } msg;

    short id = static_cast<short>(a1);
    int global_1 = *g_00939284;
    msg.id = id;
    msg.global_1 = global_1;

    unsigned long time = timeGetTime();
    int global_2 = *g_0093e8bc;
    char *src = a3;
    msg.global_2 = global_2;
    msg.time = time;
    int a2_val = a2;
    char *dest = msg.text;
    msg.a2 = a2_val;
    strcpy(dest, src);

    ++g_0093f664;
    reinterpret_cast<NetDaemon *>(g_0093cd90)
        ->send_message(reinterpret_cast<char *>(&msg), 0x2c, a4);
    --g_0093f664;
}
