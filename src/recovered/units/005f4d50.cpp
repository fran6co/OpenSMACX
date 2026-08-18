// ORIGINAL: 0x005F4D50 sub_5f4d50 0x005F4D50-0x005F4D90 FILE BYTE_EXACT
// size      64 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F4D58 0x005F4D6C 0x005F4D81
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F4D50
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f4d50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5f4d50  at 0x005F4D50  (64 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006692c0 = (int *)0x006692C0;
static int *const g_006692c4 = (int *)0x006692C4;
static int *const g_0066934c = (int *)0x0066934C;
static int *const g_009b7b40 = (int *)0x009B7B40;
static int *const g_009b7b44 = (int *)0x009B7B44;

// g_0066934c/g_006692c4/g_006692c0 are IAT slots (GetWindowLongA, GetParent,
// SetParent): `call dword ptr [addr]` in the original, reproduced by casting
// the slot to a function-pointer type and calling through the dereference -
// same idiom as src/recovered/005f12b0.cpp's GetWindowLongFn. a2 is read
// nowhere (ret 8 pops it unused).
int __stdcall sub_5f4d50(int a1, int a2) {
    typedef long (__stdcall *GetWindowLongFn)(int, int);
    typedef int (__stdcall *GetParentFn)(int);
    typedef int (__stdcall *SetParentFn)(int, int);

    int obj = (*reinterpret_cast<GetWindowLongFn *>(g_0066934c))(a1, -0x15);
    if (obj != 0 && (*reinterpret_cast<unsigned char *>(obj + 0x98) & 0x20) == 0) {
        int parent = (*reinterpret_cast<GetParentFn *>(g_006692c4))(a1);
        if (parent == *g_009b7b40) {
            (*reinterpret_cast<SetParentFn *>(g_006692c0))(a1, *g_009b7b44);
        }
    }
    return 1;
}
