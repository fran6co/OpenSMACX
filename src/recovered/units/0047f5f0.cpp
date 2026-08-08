// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0047F5F0
// name           ?pick_clouds@NetWin@@QAEXXZ
// size           1028 bytes
// measured tier  NO_COMPILE
// refusal        u0047f5f0.cpp(43) : error C2059: syntax error : ';' u0047f5f0.cpp(154) : error C2664: 'item' : cannot convert parameter 1 from 'char *' to 'struct int8 *' Types
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0047f5f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pick_clouds@NetWin@@QAEXXZ  at 0x0047F5F0  (1028 bytes)
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

struct __cdecl;
struct int8;
struct lpString;
struct position;

// ---- callees, declared and never defined (a definition would be inlined) ----
class AlphaNet { public:
    int pid_2_idx(unsigned int);
};
class BaseButton { public:
    ~BaseButton();
};
class BasePop { public:
    void close();
};
class Dialog { public:
    ~Dialog();
};
class Dialogs { public:
    int item(int8* lpString, int position);
    ~Dialogs();
};
class FlatButton { public:
    void close();
    ~FlatButton();
};
class GraphicWin { public:
    ~GraphicWin();
};
class Heap { public:
    void shutdown();
};
class Net { public:
    int send(void*, int, unsigned int, int);
};
class PopMenu { public:
    int init();
    void exec(int, int, int (__cdecl *)());
};
class Popup { public:
    Popup();
    void close();
};
class PullDown { public:
    PullDown();
    ~PullDown();
};
class Scroll { public:
    void close();
};
class Spot { public:
    ~Spot();
};
class Sprite { public:
    void close();
};
class Strings { public:
    int get(int);
};
class Win { public:
    void client_to_screen(int*, int*);
    void get_mouse_pos(int*, int*);
};
int __cdecl __alloca_probe();
int __cdecl sub_4066c0();
int __cdecl sub_406820();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00483240 = (int *)0x00483240;
static int *const g_00657453 = (int *)0x00657453;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006695c0 = (int *)0x006695C0;
static int *const g_006695c8 = (int *)0x006695C8;
static int *const g_0066974c = (int *)0x0066974C;
static int *const g_00669754 = (int *)0x00669754;
static int *const g_006698cc = (int *)0x006698CC;
static int *const g_006698d4 = (int *)0x006698D4;
static int *const g_00669d50 = (int *)0x00669D50;
static int *const g_00669d58 = (int *)0x00669D58;
static int *const g_0066a730 = (int *)0x0066A730;
static int *const g_0066a738 = (int *)0x0066A738;
static int *const g_0066a8a0 = (int *)0x0066A8A0;
static int *const g_0066a8a8 = (int *)0x0066A8A8;
static int *const g_00676ec8 = (int *)0x00676EC8;
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_0090e8ea = (int *)0x0090E8EA;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_009b3374 = (int *)0x009B3374;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class NetWin { public:
    void pick_clouds();
};

void NetWin::pick_clouds() {
    Popup popup;
    PullDown pulldown;

    reinterpret_cast<PopMenu *>(&popup)->init();

    char *const table = *reinterpret_cast<char **>(g_009b90f8);

    int idx = 0;
    int text = reinterpret_cast<Strings *>(g_009b90d8)->get(
        *reinterpret_cast<int *>(table + 0xfe4));
    reinterpret_cast<Dialogs *>(&pulldown)->item(
        reinterpret_cast<char *>(text), idx);

    for (int offset = 0xf84; offset < 0xf90; offset += 4) {
        ++idx;
        text = reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(table + offset));
        reinterpret_cast<Dialogs *>(&pulldown)->item(
            reinterpret_cast<char *>(text), idx);
    }

    int mouse_x, mouse_y;
    reinterpret_cast<Win *>(this)->get_mouse_pos(&mouse_x, &mouse_y);
    reinterpret_cast<Win *>(this)->client_to_screen(&mouse_x, &mouse_y);

    reinterpret_cast<PopMenu *>(&popup)->exec(
        mouse_x, mouse_y, reinterpret_cast<int (__cdecl *)()>(g_00483240));

    int idx2 = reinterpret_cast<AlphaNet *>(g_0093cd90)->pid_2_idx(
        *reinterpret_cast<unsigned long *>(g_0093d4f0));

    struct Packet {
        short cmd;
        int payload[6];
    } packet;
    packet.cmd = 0x2f02;
    for (int i = 0; i < 6; ++i) {
        packet.payload[i] = reinterpret_cast<int *>(g_0090e8e0)[i];
    }
    reinterpret_cast<Net *>(g_0093cd90)->send(
        &packet, 0x28, *reinterpret_cast<unsigned long *>(g_0093d4f4), 1);

    pulldown.~PullDown();
    popup.close();

    Scroll scroll;
    scroll.close();

    FlatButton fb1;
    fb1.close();
    BaseButton bb1;
    bb1.~BaseButton();

    FlatButton fb2;
    fb2.close();
    BaseButton bb2;
    bb2.~BaseButton();

    reinterpret_cast<GraphicWin *>(&scroll)->~GraphicWin();

    reinterpret_cast<BasePop *>(&popup)->close();

    Spot spot;
    spot.~Spot();

    Dialogs dialogs;
    dialogs.~Dialogs();

    Dialog dialog;
    dialog.~Dialog();

    reinterpret_cast<GraphicWin *>(&dialog)->~GraphicWin();

    sub_4066c0();
    sub_406820();

    Sprite sprite;
    sprite.close();

    Heap heap;
    heap.shutdown();

    reinterpret_cast<GraphicWin *>(&popup)->~GraphicWin();
}
