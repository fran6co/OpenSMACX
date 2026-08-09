// ORIGINAL: 0x004838B0 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004838B0
// name           ??__Fg_NEWTECHWIN_SPRITES@@YAXXZ
// size           40 bytes
// measured tier  NO_COMPILE
// refusal        u004838b0.cpp(44) : error C4234: nonstandard extension used : '__thiscall' keyword reserved for future use
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004838b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??__Fg_NEWTECHWIN_SPRITES@@YAXXZ  at 0x004838B0  (40 bytes)
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
void __stdcall fn_006456e4(void*, unsigned int size, int count, void (__thiscall *)(void*));

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00406850 = (int *)0x00406850;
static int *const g_00822604 = (int *)0x00822604;
static int *const g_00822610 = (int *)0x00822610;

// BEST EFFORT, NOT BYTE_EXACT - see the agent report for this address.
// `??__Fg_NEWTECHWIN_SPRITES@@YAXXZ` is the guarded, atexit-registered
// teardown closure for a 6-element `NewTechSprite` array (element size 0x2c,
// per-element teardown thunk at 0x406850 - the same address `src/
// atexit_thunks.cpp` binds as `SpriteElementTeardown`); the callee
// `??_M@YGXPAXIHP6EX0@Z@Z` demangles to "eh vector destructor iterator", not
// a constructor iterator. Confirmed twice (a function-local magic static
// with a non-trivial dtor, and a file-scope static array with only a
// non-trivial dtor) that VC6 ALWAYS emits this exact closure - same guard
// idiom `mov cl,[addr]; mov al,1; test al,cl; jne`, same call - under
// IMAGE_SYM_CLASS_STATIC (internal) linkage, never external, regardless of
// the array's own linkage. `object_code()` requires exactly one EXTERNAL
// .text symbol, so this exact closure is unselectable from a standalone
// translation unit under the current verifier - a structural gap shared by
// every `??__F...` / `??__E...` row, not a body defect.
typedef void (*teardown_fn)(void *);

void __cdecl fn_004838b0() {
    unsigned char *guard = reinterpret_cast<unsigned char *>(g_00822604);
    if (!(*guard & 1)) {
        *guard = static_cast<unsigned char>(*guard | 1);
        fn_006456e4(g_00822610, 0x2c, 6,
                    reinterpret_cast<teardown_fn>(g_00406850));
    }
}
