// ORIGINAL: 0x004E3300 FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004E3300
// name           sub_4e3300
// size           75 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e3300/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4e3300  at 0x004E3300  (75 bytes)
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
extern "C" void __cdecl free(void *);

// `ecx` does real work with no stack access at entry (`lea edi,[ecx-0x38]`
// before any `[ebp+...]` read) - the receiver is `this`, not a stack arg, so
// this is a __thiscall member. sub_4e3350 is called the same way (ecx=this,
// no stack args) right after, so it is a sibling member on the same class -
// this reads like a scalar deleting destructor: call the real destructor,
// then free the block and clear a tracking global if the low bit of the
// flag argument is set.
class ClassX { public:
    void sub_4e3350();
    int sub_4e3300(unsigned int a1);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b3374 = (int *)0x009B3374;

int ClassX::sub_4e3300(unsigned int a1) {
    // Original computes `outer` from `this` FIRST, then re-derives the
    // (numerically identical) `this`-as-secondary-subobject pointer as
    // `outer + 0x38` rather than keeping the incoming pointer directly -
    // reproduce that computation path, not just the equivalent value.
    char *outer = reinterpret_cast<char *>(this) - 0x38;
    ClassX *sub = reinterpret_cast<ClassX *>(outer + 0x38);
    sub->sub_4e3350();
    *reinterpret_cast<int *>(sub) = 0x6693ac;
    int field4 = *reinterpret_cast<int *>(reinterpret_cast<char *>(sub) + 4);
    *g_009b3374 = field4;
    if (a1 & 1) {
        if (outer != 0) {
            if (field4 == 0) {
                free(outer);
            }
            *g_009b3374 = 0;
        }
    }
    return reinterpret_cast<int>(outer);
}
