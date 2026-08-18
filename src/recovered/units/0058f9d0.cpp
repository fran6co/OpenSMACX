// ORIGINAL: 0x0058F9D0 ?get_pact@@YAPADH@Z 0x0058F9D0-0x0058FA19 FILE
// size      73 bytes
// prototype 
// callers   8   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0058F9D0
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0058f9d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_pact@@YAPADH@Z  at 0x0058F9D0  (73 bytes)
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

struct int8;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Strings { public:
    int get(int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00946a50 = (int *)0x00946A50;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// 0x00946A50 is indexed by a1*359 - the address itself does arithmetic here
// (base + index*4), so the const-pointer spelling would fold the literal and
// lose the lea/shl/sub sequence. Declared as an extern array instead.
extern int g_pact_table[];

// MISMATCH, closest reached: first divergence at instruction #3. Original
// zeroes edx (xor edx,edx) BEFORE storing g_009bbff0=0; every source order
// and every spelling tried here (inline, named `flag`, named `off`, a char*
// byte-offset form) compiles that store first and the xor second instead -
// looks like a VC6 scheduling/canonicalisation choice, not something this
// source shape controls. A second, separate divergence follows at
// instructions #10/#15: the original keeps `add edx, 0xc9` as its own
// instruction before `mov ecx, [eax+edx*4]`, where every spelling tried here
// gets the optimizer to fold the 0xc9 into the addressing displacement
// instead (`[eax+edx*4+0x324]`) and also hoists `mov ecx, 0x9b90d8` earlier
// than the original issues it. Tried: inline expression, a named boolean
// `flag`, a named `off` combining the constant with the flag, and rewriting
// the base as `char *` with a manual `*4` multiply - all four produced
// byte-identical output at this divergence, so it reads as an optimizer-level
// canonicalisation this call shape always hits rather than a fixable
// source-order mistake.
int8 *__cdecl get_pact(int a1) {
    *g_009bbff0 = 0;
    int val = g_pact_table[a1 * 359];
    *g_009bbfec = val;
    int *base = *reinterpret_cast<int **>(g_009b90f8);
    int off = 0xc9 + (val != 0);
    int arg = base[off];
    Strings *s = reinterpret_cast<Strings *>(g_009b90d8);
    return reinterpret_cast<int8 *>(s->get(arg));
}
