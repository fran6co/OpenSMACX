// ORIGINAL: 0x00590800 BYTE_EXACT FILE
// name      ?say_chassis@@YAXPAXHH@Z
// size      269 bytes
// spans     0x00590800-0x0059090D
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645470 0x0064FC88
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00590800
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00590800/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_chassis@@YAXPAXHH@Z  at 0x00590800  (269 bytes)
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
class Strings { public:
    int get(int);
};
int __cdecl __itoa();
int __cdecl _strcat();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

// Respelled from the scaffold's stale, unsettled declarations
// (`int __cdecl __itoa();`, `int __cdecl _strcat();` - both arity-unknown
// fallbacks, and the first even the wrong CRT name) to the real CRT
// signatures the disassembly proves: itoa(value, buffer, radix) and
// strcat(dest, src). `#pragma function` stops VC6 recognising `strcat`
// as an intrinsic and inlining a byte-copy loop in its place.
extern "C" char *strcat(char *, const char *);
#pragma function(strcat)
extern "C" char *itoa(int, char *, int);

// Three separate chassis-record fields at their own fixed addresses,
// proven by three distinct immediates all reached through the SAME
// per-record byte offset - `lea edi,[eax+eax*8]; shl edi,4` is
// `index*9*16` = `index*0x90` - added to each field's own base rather
// than to a shared struct pointer, matching the disassembly's
// `[edi + K]` addressing on every access (K different per field).
extern uint32_t g_0094a330;
extern uint8_t g_0094a378;
extern uint8_t g_0094a379;

void __cdecl say_chassis(void* a1, int a2, int a3) {
    char *dest = reinterpret_cast<char *>(a1);
    int off = a2 * 0x90;
    uint32_t name0 = *reinterpret_cast<uint32_t *>(
        reinterpret_cast<char *>(&g_0094a330) + off);
    strcat(dest, reinterpret_cast<char *>(
        reinterpret_cast<Strings *>(g_009b90d8)->get(name0)));
    if (a3 == 0) {
        return;
    }
    strcat(dest, reinterpret_cast<char *>(g_00682820));
    strcat(dest, reinterpret_cast<char *>(g_00682e9c));
    int field140 = *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x140);
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(
        reinterpret_cast<Strings *>(g_009b90d8)->get(field140)));
    strcat(dest, reinterpret_cast<char *>(g_00682e94));
    char buffer[0x50];
    uint8_t stat = *(reinterpret_cast<char *>(&g_0094a378) + off);
    itoa(stat, buffer, 10);
    strcat(dest, buffer);
    uint8_t category = *(reinterpret_cast<char *>(&g_0094a379) + off);
    int value;
    switch (category) {
    case 1:
        strcat(dest, reinterpret_cast<char *>(g_00682820));
        value = *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x28c);
        break;
    case 2:
        strcat(dest, reinterpret_cast<char *>(g_00682820));
        value = *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x288);
        break;
    default:
        goto tail;
    }
    strcat(dest, reinterpret_cast<char *>(
        reinterpret_cast<Strings *>(g_009b90d8)->get(value)));
tail:
    strcat(dest, reinterpret_cast<char *>(g_00682e98));
}
