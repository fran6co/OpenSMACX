// ORIGINAL: 0x004E3890 BYTE_EXACT FILE
// name      sub_4e3890
// size      75 bytes
// spans     0x004E3890-0x004E38DB
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3730 0x00644EF2
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004E3890
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e3890/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4e3890  at 0x004E3890  (75 bytes)
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
// The scaffold's placeholders are 0-arg stubs; the disassembly pushes an
// argument before each call, so both are redeclared under names that don't
// collide with the given zero-arg stubs. `free` is the real CRT symbol
// (`_free` after cdecl's automatic underscore); `sub_4e3730` is called with
// `this` as its thiscall receiver, so it is expressed as a sibling member
// on the same opaque class rather than the free-function stub above.
extern "C" void __cdecl free(void *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_009b3374 = (int *)0x009B3374;

// __thiscall: [ecx-0x24] is read with no stack access to compute it, so ecx
// is `this`; ret 4 pops the one stack argument. This is not a free
// __stdcall function despite the scaffold's guess.
class Obj4e3890 {
public:
    int sub_4e3730();
    void *sub_4e3890(unsigned int a1);
};

void *Obj4e3890::sub_4e3890(unsigned int a1) {
    // `this` sits 0x24 bytes into a larger allocation; `outer` recovers its
    // base. `self` is recomputed as outer+0x24 (rather than used as `this`
    // directly) because the original does exactly that via a second `lea`,
    // then reloads it into ecx for the call below.
    char *outer = reinterpret_cast<char *>(this) - 0x24;
    Obj4e3890 *self = reinterpret_cast<Obj4e3890 *>(outer + 0x24);
    self->sub_4e3730();
    *reinterpret_cast<void **>(self) = g_006693ac;
    int field = *reinterpret_cast<int *>(reinterpret_cast<char *>(self) + 4);
    *g_009b3374 = field;
    if (a1 & 1) {
        if (outer != 0) {
            if (field == 0) {
                free(outer);
            }
            *g_009b3374 = 0;
        }
    }
    return outer;
}
