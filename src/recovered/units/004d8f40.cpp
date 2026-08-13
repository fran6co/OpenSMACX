// ORIGINAL: 0x004D8F40 BYTE_EXACT FILE
// name      ?explore@Console@@QAEXH@Z
// size      96 bytes
// spans     0x004D8F40-0x004D8FA0
// prototype void (__thiscall ?explore@Console@@QAEXH@Z)(Console* this, int vehID)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00531020 0x005310F0 0x00531480 0x00593220
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004D8F40
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004d8f40/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?explore@Console@@QAEXH@Z  at 0x004D8F40  (96 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int lock_veh(int*, int, int, int, int);
    void await_synch();
    void unlock_veh();
};
void __cdecl synch_veh(int vehID);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0093cd90 = (int *)0x0093CD90;

class Console { public:
    void explore(int vehID);
};

// Shadow struct: one array of these at a fixed base, stride 0x34 -
// a dword flags field at offset 0 and a byte field at offset 0xD, proven
// by the two addressing constants (0x95282C and 0x952839 = base + 0xD)
// sharing the same `index * 0x34` scale.
struct VehRecord {
    uint32_t flags_;
    uint8_t pad_4_[0xD - 4];
    uint8_t byte_d_;
    uint8_t pad_e_[0x34 - 0xD - 1];
};

extern VehRecord g_vehRecords[];

void Console::explore(int a1) {
    int result = reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_veh(&a1, 0, -1, -1, 0);
    if (result == 0) {
        g_vehRecords[a1].byte_d_ = 0;
        g_vehRecords[a1].flags_ |= 0x4000;
        synch_veh(a1);
        reinterpret_cast<NetDaemon *>(g_0093cd90)->await_synch();
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
    }
}
