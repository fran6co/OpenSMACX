// ORIGINAL: 0x005906F0 FILE
// name      ?say_armor@@YAXPAXHH@Z
// size      261 bytes
// spans     0x005906F0-0x005907F5
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645470 0x0064FC88
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005906F0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005906f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_armor@@YAXPAXHH@Z  at 0x005906F0  (261 bytes)
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
extern "C" char* __cdecl _itoa(int value, char* str, int radix);
// Named off the standard CRT prototype: MSVC recognizes an identifier
// spelled `strcat` under /O2 and substitutes its own inline scan+copy
// (repne scasb + rep movsd), which the original call sites do not do.
// A differently-named extern "C" declaration keeps the real object-file
// call - the relocation target name itself is masked by the comparator.
extern "C" char* __cdecl crt_strcat(char*, const char*);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_0068faec = (int *)0x0068FAEC;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
// Not caught by the emitter's absolute-operand scan: it appears only as
// a [reg+disp32] displacement (`[edi + 0x94f278]`), not a bare address.
// A 16-byte-stride table; field+0 is an int string id, +8 a signed
// armor byte, +9 an unsigned ammo-symbol count.
static char *const g_0094f278 = (char *)0x0094F278;

void __cdecl say_armor(void* a1, int a2, int a3) {
    char *dst = reinterpret_cast<char *>(a1);
    int base = a2 * 16;
    crt_strcat(dst, reinterpret_cast<char *>(
        reinterpret_cast<Strings *>(g_009b90d8)->get(*reinterpret_cast<int *>(g_0094f278 + base))));
    if (a3 != 0) {
        crt_strcat(dst, reinterpret_cast<char *>(g_00682820));
        crt_strcat(dst, reinterpret_cast<char *>(g_00682e9c));
        int field = *reinterpret_cast<int *>(*g_009b90f8 + 0x154);
        int val = reinterpret_cast<Strings *>(g_009b90d8)->get(field);
        crt_strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(val));
        crt_strcat(dst, reinterpret_cast<char *>(g_00682e94));
        signed char armor = *reinterpret_cast<signed char *>(g_0094f278 + base + 8);
        if (armor < 0) {
            int field2 = *reinterpret_cast<int *>(*g_009b90f8 + 0x310);
            int val2 = reinterpret_cast<Strings *>(g_009b90d8)->get(field2);
            crt_strcat(dst, reinterpret_cast<char *>(val2));
        } else {
            char buf[80];
            _itoa(armor, buf, 10);
            crt_strcat(dst, buf);
        }
        int i = 0;
        if (*reinterpret_cast<unsigned char *>(g_0094f278 + base + 9) > 0) {
            do {
                crt_strcat(dst, reinterpret_cast<char *>(g_0068faec));
                ++i;
            } while (i < *reinterpret_cast<unsigned char *>(g_0094f278 + base + 9));
        }
        crt_strcat(dst, reinterpret_cast<char *>(g_00682e98));
    }
}
