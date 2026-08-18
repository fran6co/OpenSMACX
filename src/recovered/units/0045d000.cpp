// ORIGINAL: 0x0045D000 ?on_iface_mouse_move@MainInterface@@QAEXHH@Z 0x0045D000-0x0045D105 FILE
// size      261 bytes
// prototype void (__thiscall ?on_iface_mouse_move@MainInterface@@QAEXHH@Z)(MainInterface* this, int xCoord, int yCoord)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045C3A0 0x004B4CF0 0x004B6530 0x005FAB00
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045D000
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045d000/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_iface_mouse_move@MainInterface@@QAEXHH@Z  at 0x0045D000  (261 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class Spot { public:
    int check(int xCoord, int yCoord, int* spotPos, int* spotType);
};
class StatusWin { public:
    void draw_active(int, int, int);
    void reset_active();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_008c5568 = (int *)0x008C5568;
static int *const g_008c6b2c = (int *)0x008C6B2C;
static int *const g_008c6b30 = (int *)0x008C6B30;
static int *const g_008c6b48 = (int *)0x008C6B48;

class MainInterface { public:
    void redraw_complete();
    void on_iface_mouse_move(int xCoord, int yCoord);
};

void MainInterface::on_iface_mouse_move(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    if (reinterpret_cast<Spot *>(self + 0x24620)->check(a1, a2, &a2, &a1) != -1
            && a2 == 5) {
        if (*reinterpret_cast<int *>(self + 0x20cd8) != 0) {
            return;
        }
        *reinterpret_cast<int *>(self + 0x20cd8) = 1;
        reinterpret_cast<MainInterface *>(self - 0xa14)->redraw_complete();
        return;
    }

    int check2 = reinterpret_cast<Spot *>(g_008c6b48)->check(a1, a2, &a2, &a1);
    if (check2 >= 0 && a1 == 0xb) {
        if (a2 != *g_008c6b30) {
            *g_008c6b30 = a2;
            *g_008c6b2c = -1;
            reinterpret_cast<StatusWin *>(g_008c5568)->draw_active(-1, a2, 0);
        }
    } else if (check2 >= 0 && a1 == 0) {
        if (a2 != *g_008c6b2c) {
            *g_008c6b30 = -1;
            *g_008c6b2c = a2;
            reinterpret_cast<StatusWin *>(g_008c5568)->draw_active(a2, -1, 0);
        }
    } else {
        reinterpret_cast<StatusWin *>(g_008c5568)->reset_active();
    }

    if (*reinterpret_cast<int *>(self + 0x20cd8) == 1) {
        *reinterpret_cast<int *>(self + 0x20cd8) = 0;
        reinterpret_cast<MainInterface *>(self - 0xa14)->redraw_complete();
    }
}
