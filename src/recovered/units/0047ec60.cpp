// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0047EC60
// name           ?pick_ocean@NetWin@@QAEXXZ
// size           1028 bytes
// measured tier  NO_COMPILE
// refusal        u0047ec60.cpp(43) : error C2059: syntax error : ';' u0047ec60.cpp(167) : error C2664: 'item' : cannot convert parameter 1 from 'char *' to 'struct int8 *' Types
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0047ec60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pick_ocean@NetWin@@QAEXXZ  at 0x0047EC60  (1028 bytes)
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
static int *const g_006571cb = (int *)0x006571CB;
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
static int *const g_00676ca0 = (int *)0x00676CA0;
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_0090e8e7 = (int *)0x0090E8E7;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_009b3374 = (int *)0x009B3374;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class NetWin { public:
    void pick_ocean();
};

void NetWin::pick_ocean() {
    // `Popup` is a ~0x6300-byte local aggregate (a dialog with many nested
    // widgets) that the context leaves opaque; a raw byte buffer stands in
    // for it and every nested widget is reached by the byte offset the
    // disassembly shows relative to its start.
    char popup_storage[0x6300];
    char *popup = popup_storage;
    Popup *popup_obj = reinterpret_cast<Popup *>(popup);
    *popup_obj = Popup();

    PullDown *pulldown = reinterpret_cast<PullDown *>(popup + 0x537C);
    *pulldown = PullDown();

    *reinterpret_cast<int *>(popup) = reinterpret_cast<int>(g_0066a8a8);
    *reinterpret_cast<int *>(popup + 0x744) = reinterpret_cast<int>(g_0066a8a0);
    *reinterpret_cast<int *>(popup + 0x537C) = reinterpret_cast<int>(g_0066a738);
    *reinterpret_cast<int *>(popup + 0x57C0) = reinterpret_cast<int>(g_0066a730);

    reinterpret_cast<PopMenu *>(popup)->init();

    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
    Dialogs *dialogs = reinterpret_cast<Dialogs *>(popup + 0x21D0);
    char *table = *reinterpret_cast<char **>(g_009b90f8);
    int idx = 0;
    int v = strings->get(*reinterpret_cast<int *>(table + 0xfe4));
    dialogs->item(reinterpret_cast<char *>(v), idx);
    int off = 0xf60;
    do {
        char *table2 = *reinterpret_cast<char **>(g_009b90f8);
        int w = *reinterpret_cast<int *>(table2 + off);
        int v2 = strings->get(w);
        idx++;
        dialogs->item(reinterpret_cast<char *>(v2), idx);
        off += 4;
    } while (off < 0xf6c);

    int x = 0, y = 0;
    Win *win = reinterpret_cast<Win *>(this);
    win->get_mouse_pos(&x, &y);
    win->client_to_screen(&x, &y);
    reinterpret_cast<PopMenu *>(popup)->exec(
        x, y, reinterpret_cast<int (__cdecl *)()>(g_00483240));

    pulldown->~PullDown();
    *reinterpret_cast<int *>(popup) = reinterpret_cast<int>(g_006695c8);
    *reinterpret_cast<int *>(popup + 0x744) = reinterpret_cast<int>(g_006695c0);
    popup_obj->close();

    char *scroll = popup + 0x3230;
    *reinterpret_cast<int *>(scroll) = reinterpret_cast<int>(g_00669d58);
    *reinterpret_cast<int *>(scroll + 0x444) = reinterpret_cast<int>(g_00669d50);
    reinterpret_cast<Scroll *>(scroll)->close();

    char *fb1 = popup + 0x4828;
    *reinterpret_cast<int *>(fb1) = reinterpret_cast<int>(g_00669754);
    *reinterpret_cast<int *>(fb1 + 0x444) = reinterpret_cast<int>(g_0066974c);
    reinterpret_cast<FlatButton *>(fb1)->close();
    reinterpret_cast<BaseButton *>(fb1)->~BaseButton();

    char *fb2 = popup + 0x3CDC;
    *reinterpret_cast<int *>(fb2) = reinterpret_cast<int>(g_00669754);
    *reinterpret_cast<int *>(fb2 + 0x444) = reinterpret_cast<int>(g_0066974c);
    reinterpret_cast<FlatButton *>(fb2)->close();
    reinterpret_cast<BaseButton *>(fb2)->~BaseButton();

    reinterpret_cast<GraphicWin *>(popup)->~GraphicWin();

    *reinterpret_cast<int *>(popup) = reinterpret_cast<int>(g_006698d4);
    *reinterpret_cast<int *>(popup + 0x744) = reinterpret_cast<int>(g_006698cc);
    reinterpret_cast<BasePop *>(popup)->close();

    reinterpret_cast<Spot *>(popup + 0x3098)->~Spot();
    reinterpret_cast<Dialogs *>(popup + 0x2358)->~Dialogs();
    reinterpret_cast<Dialog *>(popup + 0x2D70)->~Dialog();
    reinterpret_cast<GraphicWin *>(popup + 0x2358)->~GraphicWin();

    int32_t ecx_val = *reinterpret_cast<int32_t *>(popup + 0x21AC);
    *reinterpret_cast<int32_t *>(popup + 0x21A8) = reinterpret_cast<int32_t>(g_006693ac);
    *g_009b3374 = ecx_val;

    sub_406820();

    reinterpret_cast<Sprite *>(popup + 0x2118)->close();
    reinterpret_cast<FlatButton *>(popup + 0x15A8)->~FlatButton();
    reinterpret_cast<FlatButton *>(popup + 0xA5C)->~FlatButton();
    reinterpret_cast<Heap *>(popup + 0xA28)->shutdown();
    reinterpret_cast<GraphicWin *>(popup)->~GraphicWin();

    unsigned int netid = *reinterpret_cast<unsigned int *>(g_0093d4f0);
    reinterpret_cast<AlphaNet *>(g_0093cd90)->pid_2_idx(netid);

    char buf[0x28];
    for (int i = 0; i < 6; i++) {
        reinterpret_cast<int32_t *>(buf + 0x10)[i] = g_0090e8e0[i];
    }
    *reinterpret_cast<short *>(buf) = 0x2f02;
    unsigned int chan = *reinterpret_cast<unsigned int *>(g_0093d4f4);
    reinterpret_cast<Net *>(g_0093cd90)->send(buf, 0x28, chan, 1);
}
