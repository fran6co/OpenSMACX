// ORIGINAL: 0x0055D190 FILE
// name      ?pact_treaty@@YAXHH@Z
// size      259 bytes
// spans     0x0055D190-0x0055D293
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x0058FCE0 0x00625EC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0055D190
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0055d190/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pact_treaty@@YAXHH@Z  at 0x0055D190  (259 bytes)
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

struct int8;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetMsg { public:
    void pop(int8*, int, int, int8*);
};
int __cdecl parse_says(int nID, int8* input, int gender, int pluralality);
int8* __cdecl get_he_she(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068e62c = (int *)0x0068E62C;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// Respelled from the scaffold's `int8*` (an incomplete forward-declared
// struct in this unit): the catalogue's free-function call target mangles
// this parameter as `char*` (PAD), so this is a correctly-typed overload
// declared alongside the stale one - see the emitter's note on int8/char.
// `get_he_she` keeps the scaffold's `int8*`-returning declaration: a second
// one differing only in return type is not a valid overload, so its result
// is cast at the call site instead.
int __cdecl parse_says(int, char *, int, int);

// Shadow struct: one record array, stride 0x59c bytes (proven by the
// lea/lea/shl/sub/shl chain computing `index*5*9*8 - index`, i.e.
// `index*0x167` dwords = `index*0x59c` bytes), with two int fields and
// three char-buffer fields at the offsets the disassembly's constants give.
// Reached through a raw byte pointer rather than `array[index]` so the
// element size is not visible as a compile-time constant to fold into a
// SIB scale.
struct PactRecord {
    uint32_t field_0_;
    uint8_t pad_4_[0x34 - 4];
    char field_34_[0x4C - 0x34];
    char field_4C_[0x2E4 - 0x4C];
    char field_2E4_[0x2FC - 0x2E4];
    uint32_t field_2FC_;
    uint32_t field_300_;
    uint8_t pad_304_[0x59c - 0x304];
};

extern uint8_t g_pactRecords[];

void __cdecl pact_treaty(int a1, int a2) {
    *g_009bbff0 = 0;
    PactRecord &rec1 = *reinterpret_cast<PactRecord *>(
        g_pactRecords + a1 * static_cast<int>(sizeof(PactRecord)));
    *g_009bbfec = rec1.field_0_;
    parse_says(0, rec1.field_4C_, -1, -1);
    *g_009bbfec = rec1.field_0_;
    *g_009bbff0 = 0;
    parse_says(1, rec1.field_34_, -1, -1);
    *g_009bbfec = rec1.field_2FC_;
    *g_009bbff0 = rec1.field_300_;
    parse_says(2, rec1.field_2E4_, -1, -1);
    parse_says(3, reinterpret_cast<char *>(get_he_she(a1, 0)), -1, -1);
    PactRecord &rec2 = *reinterpret_cast<PactRecord *>(
        g_pactRecords + a2 * static_cast<int>(sizeof(PactRecord)));
    *g_009bbfec = rec2.field_2FC_;
    *g_009bbff0 = rec2.field_300_;
    parse_says(4, rec2.field_2E4_, -1, -1);
    reinterpret_cast<NetMsg *>(g_00805338)->pop(
        reinterpret_cast<int8 *>(g_0068e62c), 5000, 0, 0);
}
