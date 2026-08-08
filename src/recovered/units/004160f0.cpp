// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004160F0
// name           ?iface_click@BaseWin@@QAEXHHHH@Z
// size           1058 bytes
// measured tier  MISMATCH
// divergence     77
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004160f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?iface_click@BaseWin@@QAEXHHHH@Z  at 0x004160F0  (1058 bytes)
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

struct RECT;
struct int8;
struct nCursorName;
struct spotPos;
struct spotType;
struct vehID;
struct xCoord;
struct yCoord;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int lock_base(int, int, int, int);
    void unlock_base(int);
};
class Spot { public:
    int check(int xCoord, int yCoord, int* spotPos, int* spotType);
};
class Strings { public:
    int get(int);
};
class TutWin { public:
    int reset();
};
class Win { public:
    int is_visible();
    int set_cursor(int nCursorName);
    void client_to_screen(int*, int*);
    void set_bubble_text(int8*, RECT*);
};
int __cdecl base_compute(int);
int __cdecl in_box(int, int, RECT*);
void __cdecl set_base(int baseID);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 4
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00416514 = (int *)0x00416514;
static int *const g_00689370 = (int *)0x00689370;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8c8 = (int *)0x0093E8C8;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0097d0b8 = (int *)0x0097D0B8;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009b7abc = (int *)0x009B7ABC;
static int *const g_009b7ac0 = (int *)0x009B7AC0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class BaseWin { public:
    void queue_click(int, int, int, int);
    void pop_click(int, int, int, int);
    void garrison_click(int vehID, int, int, int);
    void production(int, int);
    void iface_click(int xCoord, int yCoord, int, int);
};

void BaseWin::iface_click(int xCoord, int yCoord, int a3, int a4) {
    reinterpret_cast<TutWin *>(g_008c6e68)->reset();

    char *self = reinterpret_cast<char *>(this);
    Win *thisWin = reinterpret_cast<Win *>(this);

    if (*reinterpret_cast<int32_t *>(self + 0x40b54) > 0) {
        thisWin->set_cursor(0x7f00);
        reinterpret_cast<Win *>(g_007ae820)->set_cursor(0x7f00);
    }
    *reinterpret_cast<int32_t *>(self + 0x40b54) = -1;

    if (*g_009b7abc == reinterpret_cast<int32_t>(this)) {
        *g_009b7abc = 0;
        reinterpret_cast<VCall *>(this)->slot004();
    }
    if (*g_009b7ac0 == reinterpret_cast<int32_t>(this))
        *g_009b7ac0 = 0;

    if (!thisWin->is_visible())
        goto DONE;
    if (*g_0093f660 != 0 && *g_0093e8c8 != 0)
        goto DONE;

    {
        int spotPos;
        int spotType;
        Spot *spot = reinterpret_cast<Spot *>(self + 0x40eb4);
        if (spot->check(xCoord, yCoord, &spotPos, &spotType) < 0) {
            if (a3 != 0 &&
                in_box(xCoord, yCoord, reinterpret_cast<RECT *>(self + 0x40c5c))) {
                int x = xCoord, y = yCoord;
                int idx = *reinterpret_cast<int32_t *>(*g_009b90f8 + 0xca4);
                int str = reinterpret_cast<Strings *>(g_009b90d8)->get(idx);
                reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&x, &y);
                int32_t rect[4] = {x, y, x, y};
                reinterpret_cast<Win *>(g_007ae820)->set_bubble_text(
                    reinterpret_cast<int8 *>(str),
                    reinterpret_cast<RECT *>(rect));
                return;
            }
            if (in_box(xCoord, yCoord, reinterpret_cast<RECT *>(self + 0x40b7c))) {
                int x = xCoord, y = yCoord;
                int idx = *reinterpret_cast<int32_t *>(*g_009b90f8 + 0xc9c);
                int str = reinterpret_cast<Strings *>(g_009b90d8)->get(idx);
                reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&x, &y);
                int32_t rect[4] = {x, y, x, y};
                reinterpret_cast<Win *>(g_007ae820)->set_bubble_text(
                    reinterpret_cast<int8 *>(str),
                    reinterpret_cast<RECT *>(rect));
                return;
            }
            if (in_box(xCoord, yCoord, reinterpret_cast<RECT *>(self + 0x40c2c))) {
                int x = xCoord, y = yCoord;
                int idx = *reinterpret_cast<int32_t *>(*g_009b90f8 + 0xca0);
                int str = reinterpret_cast<Strings *>(g_009b90d8)->get(idx);
                reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&x, &y);
                int32_t rect[4] = {x, y, x, y};
                reinterpret_cast<Win *>(g_007ae820)->set_bubble_text(
                    reinterpret_cast<int8 *>(str),
                    reinterpret_cast<RECT *>(rect));
                return;
            }
            if (in_box(xCoord, yCoord, reinterpret_cast<RECT *>(self + 0x40c6c))) {
                int idx = *reinterpret_cast<int32_t *>(*g_009b90f8 + 0xcd0);
                int str = reinterpret_cast<Strings *>(g_009b90d8)->get(idx);
                int x = xCoord, y = yCoord;
                reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&x, &y);
                int32_t rect[4] = {x, y, x, y};
                reinterpret_cast<Win *>(g_007ae820)->set_bubble_text(
                    reinterpret_cast<int8 *>(str),
                    reinterpret_cast<RECT *>(rect));
            }
            if (!in_box(xCoord, yCoord, reinterpret_cast<RECT *>(self + 0x40c5c)))
                goto DONE;
            spotType = 1;
        }

        if (reinterpret_cast<Win *>(self + 0xa1c)->is_visible() && spotType != 8)
            goto DONE;

        {
            int32_t vehId = *reinterpret_cast<int32_t *>(self + 0x40b0c);
            set_base(vehId);
            int32_t cachedVal = g_0097d0b8[vehId * 77];
            base_compute(0);
            if (*g_0093f660 != 0 && *g_0093a938 == 0) {
                int32_t freshVehId = *reinterpret_cast<int32_t *>(self + 0x40b0c);
                if (cachedVal != g_0097d0b8[freshVehId * 77]) {
                    if (reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_base(
                            *g_00689370, 0, -1, -1) == 0) {
                        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_base(*g_00689370);
                    }
                }
            }

            uint8_t byteVal = *reinterpret_cast<uint8_t *>(*g_0090ea30 + 4);
            int32_t eax2 = *g_00939284;
            if (byteVal != eax2 &&
                (*reinterpret_cast<uint8_t *>(g_009a64c0) & 0x80) == 0) {
                int32_t index = byteVal + eax2 * 2099;
                if ((reinterpret_cast<uint8_t *>(g_0096c9f8)[index * 4] & 1) == 0 &&
                    spotType != 6)
                    goto DONE;
            }

            if (a4 != 0)
                goto DONE;

            int32_t dispatchArg;
            int32_t fieldF = *reinterpret_cast<int32_t *>(self + 0x40b4c);
            if (fieldF == 0 && spotType == 1) {
                spotType = 1;
                dispatchArg = -1;
                spotPos = -1;
            } else {
                dispatchArg = spotPos;
            }

            switch (spotType) {
            case 1:
                pop_click(dispatchArg, xCoord, yCoord, a3);
                return;
            case 2:
                queue_click(dispatchArg, xCoord, yCoord, a3);
                return;
            case 3:
                garrison_click(dispatchArg, xCoord, yCoord, a3);
                return;
            case 4:
                garrison_click(dispatchArg, xCoord, yCoord, a3);
                return;
            case 8:
                production(0, 0);
                break;
            default:
                break;
            }
        }
    }

DONE:;
}
