// ORIGINAL: 0x004C3D40 ?click@WorldWin@@QAEXHHHH@Z 0x004C3D40-0x004C3E48 FILE
// size      264 bytes
// prototype void (__thiscall ?click@WorldWin@@QAEXHHHH@Z)(WorldWin* this, int, int, int, int)
// callers   2   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046B1F0 0x004C3AB0 0x004C43F0 0x005EDCD0 0x0062B870
// indirect  0x004C3D72 0x004C3E33
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004C3D40
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c3d40/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?click@WorldWin@@QAEXHHHH@Z  at 0x004C3D40  (264 bytes)
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
class ButtonGroup { public:
    int set(int, int);
};
class MapWin { public:
    void set_center(int xCoord, int yCoord, int);
};
class Win { public:
    void hide();
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1. The emitter's slot001 is
// nullary but this dispatch (0x4c3e33) pushes one int, so a second
// shim spells the arg version - same "second shim" case as 0x4c39d0.
class VCall { public:
    virtual void slot000();
    virtual void slot001();  // <-- unused by this body; kept for parity
};
class VCallArg { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669330 = (int *)0x00669330;
static int *const g_006e8150 = (int *)0x006E8150;
static int *const g_007cd12c = (int *)0x007CD12C;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_007d3c40 = (int *)0x007D3C40;
static int *const g_008e9f60 = (int *)0x008E9F60;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;

class WorldWin { public:
    void right_menu(int, int);
    int pixel_to_tile(int, int, int*, int*);
    void click(int, int, int, int);
};

typedef short (__stdcall *Fn00669330)(int);

void WorldWin::click(int a1, int a2, int a3, int a4) {
    int local;
    *g_006e8150 = 0;
    if (a3 != 0) {
        right_menu(a1, a2);
        return;
    }
    short keystate = (*reinterpret_cast<Fn00669330 *>(g_00669330))(0x10);
    unsigned char highByte = reinterpret_cast<unsigned char *>(&keystate)[1];
    int flag = 1;
    if (highByte == 0) {
        flag = a4;
    }
    if (pixel_to_tile(a1, a2, &local, &a3) != 0) {
        return;
    }
    if (a3 < 0) {
        return;
    }
    if (a3 >= *g_00949874) {
        return;
    }
    if (local < 0) {
        return;
    }
    if (local >= *g_00949870) {
        return;
    }
    if (flag != 0) {
        char *p = reinterpret_cast<char *>(*g_007d3c3c);
        int v1dda0 = *reinterpret_cast<int *>(p + 0x1dda0);
        int v1dd9c = *reinterpret_cast<int *>(p + 0x1dd9c);
        reinterpret_cast<MapWin *>(*g_007d3c40)->set_center(v1dd9c, v1dda0, 1);
        reinterpret_cast<ButtonGroup *>(g_007cd12c)->set(0x3ef, 0);
        char *self = reinterpret_cast<char *>(this);
        *reinterpret_cast<int *>(self + 0x239cc) = 1;
        char *q = *reinterpret_cast<char **>(g_007d3c40);
        char *v = *reinterpret_cast<char **>(q);
        int adj = *reinterpret_cast<int *>(v + 4);
        reinterpret_cast<VCallArg *>(q + adj)->slot001(0);
        reinterpret_cast<Win *>(g_008e9f60)->hide();
    } else {
        reinterpret_cast<MapWin *>(*g_007d3c3c)->set_center(local, a3, 1);
    }
}
