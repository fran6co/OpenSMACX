// ORIGINAL: 0x00592E10 ?message_veh@@YAXHHHH@Z 0x00592E10-0x00592E70 FILE
// size      96 bytes
// prototype void (__cdecl ?message_veh@@YAXHHHH@Z)(int, int vehID, int xCoord, int yCoord)
// callers   12   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532940
// indirect  0x00592E27
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00592E10
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00592e10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?message_veh@@YAXHHHH@Z  at 0x00592E10  (96 bytes)
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

struct vehID;
struct xCoord;
struct yCoord;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    void send_message(int8*, unsigned int, int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8bc = (int *)0x0093E8BC;
static int *const g_0093f664 = (int *)0x0093F664;

// `int vehID a2` / `int xCoord a3` / `int yCoord a4` in the emitted head was
// not valid C++ - the emitter's naming heuristic pasted a hinted struct tag
// into the type position. The contract's own function head confirms plain
// `int` for all four parameters, so that is what is written below.
typedef unsigned long (__stdcall *TimeGetTimeFn)(void);

// g_0093f664 is `in_flight_sends`, a send-in-flight counter (see the
// neighbouring src/recovered/00592de0.cpp): the static-const-pointer
// spelling folds the increment/decrement into a load/modify/store under
// /O2 where the original keeps a single RMW instruction. An `extern`
// declaration keeps the single instruction.
extern int in_flight_sends;

void __cdecl message_veh(int a1, int a2, int a3, int a4) {
    struct Msg {
        short f0;
        int f1;
        int f2;
        int f3;
        int f4;
        int f5;
    } msg;

    msg.f0 = static_cast<short>(a1);
    msg.f1 = *g_00939284;
    msg.f2 = (*reinterpret_cast<TimeGetTimeFn *>(g_00669368))();
    msg.f3 = *g_0093e8bc;
    msg.f5 = a3;
    msg.f4 = a2;

    in_flight_sends++;
    reinterpret_cast<NetDaemon *>(g_0093cd90)->send_message(
        reinterpret_cast<int8 *>(&msg), 0x18, a4);
    in_flight_sends--;
}
