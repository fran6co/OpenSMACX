// ORIGINAL: 0x004455F0 ?wave_it@@YAXH@Z 0x004455F0-0x0044564D FILE
// size      93 bytes
// prototype 
// callers   7   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6920 0x004C6B10 0x004C6CE0
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004455F0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004455f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wave_it@@YAXH@Z  at 0x004455F0  (93 bytes)
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
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Wave { public:
    int is_playing();
    int load();
    int play();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0074c5f0 = (int *)0x0074C5F0;
static int *const g_0074daa4 = (int *)0x0074DAA4;
static int *const g_00945ed0 = (int *)0x00945ED0;
static int *const g_009a6490 = (int *)0x009A6490;

// Indexed table base: the literal-address spelling above folds to an
// immediate and the compiler emits `add` instead of `lea` once it is
// combined with a scaled index. A real extern symbol keeps the `lea` -
// the relocation it needs is masked by the comparison.
extern char g_wave_table_0074c5f0[];

void __cdecl wave_it(int a1) {
    if ((*g_009a6490 & 0x400) != 0) {
        if (!reinterpret_cast<Wave *>(g_00945ed0)->is_playing()) {
            int idx = *g_0074daa4;
            Wave *cur = reinterpret_cast<Wave *>(g_wave_table_0074c5f0 + idx * 0x6C);
            if (!cur->is_playing()) {
                *g_0074daa4 = a1;
                Wave *w = reinterpret_cast<Wave *>(g_wave_table_0074c5f0 + a1 * 0x6C);
                w->load();
                w->play();
            }
        }
    }
}
