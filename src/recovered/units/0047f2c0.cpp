// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0047F2C0
// name           ?pick_native@NetWin@@QAEXXZ
// size           1028 bytes
// measured tier  NO_COMPILE
// refusal        u0047f2c0.cpp(43) : error C2059: syntax error : ';' u0047f2c0.cpp(153) : warning C4291: 'void *__cdecl operator new(unsigned int,void *)' : no matching operator
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0047f2c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pick_native@NetWin@@QAEXXZ  at 0x0047F2C0  (1028 bytes)
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
static int *const g_0065737b = (int *)0x0065737B;
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
static int *const g_00676e10 = (int *)0x00676E10;
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_0090e8e9 = (int *)0x0090E8E9;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_009b3374 = (int *)0x009B3374;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class NetWin { public:
    void pick_native();
};

inline void *__cdecl operator new(unsigned int, void *p) { return p; }

void NetWin::pick_native() {
    // Same wall as 0x004DC520: Popup is opaque (no fields) in the scaffold,
    // but its ctor is invoked directly on a ~0x6300-byte stack region, so a
    // real automatic `Popup popup;` cannot be sized to match. Raw storage +
    // placement-new is the only way to reach the later offsets; the SEH
    // prologue this function actually has is not reproduced as a result.
    unsigned char popupStorage[0x6300];
    unsigned char pulldownStorage[0x448];

    Popup *popup = new (popupStorage) Popup();

    // The compiler-inlined "derived identity" vtable pokes recur across
    // this whole widget family with the same relative offset (+0x444) -
    // see STRUCTURE. First the popup is stamped as a PullDown host...
    new (pulldownStorage) PullDown();
    *reinterpret_cast<void **>(pulldownStorage) = g_0066a738;
    *reinterpret_cast<void **>(pulldownStorage + 0x444) = g_0066a730;

    // ...then as a PopMenu, before PopMenu::init() is called on it.
    *reinterpret_cast<void **>(popupStorage) = g_0066a8a8;
    *reinterpret_cast<void **>(popupStorage + 0x444) = g_0066a8a0;
    reinterpret_cast<PopMenu *>(popupStorage)->init();

    int **table_base = reinterpret_cast<int **>(g_009b90f8);
    int first_idx = *reinterpret_cast<int *>(reinterpret_cast<char *>(*table_base) + 0xfe4);
    int first_str = reinterpret_cast<Strings *>(g_009b90d8)->get(first_idx);

    unsigned char dialogsStorage[0x400];
    Dialogs *dialogs = reinterpret_cast<Dialogs *>(dialogsStorage);
    int item_index = 0;
    dialogs->item(reinterpret_cast<int8 *>(first_str), item_index);

    for (int off = 0xf90; off < 0xf9c; off += 4) {
        int idx = *reinterpret_cast<int *>(reinterpret_cast<char *>(*table_base) + off);
        int str = reinterpret_cast<Strings *>(g_009b90d8)->get(idx);
        ++item_index;
        dialogs->item(reinterpret_cast<int8 *>(str), item_index);
    }

    int mouse_x, mouse_y;
    reinterpret_cast<Win *>(this)->get_mouse_pos(&mouse_x, &mouse_y);
    reinterpret_cast<Win *>(this)->client_to_screen(&mouse_x, &mouse_y);

    // exec() is declared void in the scaffold, but the disassembly reads
    // eax right after the call - see PROPOSALS. The pointer-to-member-
    // function cast lets the call site use the right return type without
    // redeclaring PopMenu (which would conflict with the scaffold).
    typedef int (PopMenu::*ExecFn)(int, int, int(__cdecl *)());
    ExecFn exec_fn = reinterpret_cast<ExecFn>(&PopMenu::exec);
    int pick = (reinterpret_cast<PopMenu *>(popupStorage)->*exec_fn)(
        mouse_x, mouse_y, reinterpret_cast<int(__cdecl *)()>(g_00483240));
    if (pick >= 0) {
        *reinterpret_cast<signed char *>(g_0090e8e9) = static_cast<signed char>(pick - 1);
    }

    reinterpret_cast<AlphaNet *>(g_0093cd90)->pid_2_idx(*reinterpret_cast<unsigned int *>(g_0093d4f0));

    unsigned char msgbuf[0x28];
    *reinterpret_cast<unsigned short *>(msgbuf) = 0x2f02;
    for (int i = 0; i < 6; ++i) {
        reinterpret_cast<int *>(msgbuf + 0x10)[i] = reinterpret_cast<int *>(g_0090e8e0)[i];
    }
    reinterpret_cast<Net *>(g_0093cd90)->send(msgbuf, 0x28, *reinterpret_cast<unsigned int *>(g_0093d4f4), 1);

    reinterpret_cast<PullDown *>(pulldownStorage)->~PullDown();

    *reinterpret_cast<void **>(popupStorage) = g_006695c8;
    *reinterpret_cast<void **>(popupStorage + 0x444) = g_006695c0;
    reinterpret_cast<Popup *>(popupStorage)->close();

    // The embedded panel (GraphicWin/Scroll composite) at popup+0x3230,
    // with two FlatButtons at +0xAAC and +0x15F8 - the same offsets as
    // 0x004DC520's popup, from the same class. See STRUCTURE.
    unsigned char *panel = popupStorage + 0x3230;
    *reinterpret_cast<void **>(panel) = g_00669d58;
    *reinterpret_cast<void **>(panel + 0x444) = g_00669d50;
    reinterpret_cast<Scroll *>(panel)->close();

    unsigned char *btn_a = panel + 0x15F8;
    *reinterpret_cast<void **>(btn_a) = g_00669754;
    *reinterpret_cast<void **>(btn_a + 0x444) = g_0066974c;
    reinterpret_cast<FlatButton *>(btn_a)->close();
    reinterpret_cast<BaseButton *>(btn_a)->~BaseButton();

    unsigned char *btn_b = panel + 0xAAC;
    *reinterpret_cast<void **>(btn_b) = g_00669754;
    *reinterpret_cast<void **>(btn_b + 0x444) = g_0066974c;
    reinterpret_cast<FlatButton *>(btn_b)->close();
    reinterpret_cast<BaseButton *>(btn_b)->~BaseButton();

    reinterpret_cast<GraphicWin *>(panel)->~GraphicWin();

    *reinterpret_cast<void **>(popupStorage) = g_006698d4;
    *reinterpret_cast<void **>(popupStorage + 0x444) = g_006698cc;
    reinterpret_cast<BasePop *>(popupStorage)->close();

    unsigned char spotStorage[0x10];
    reinterpret_cast<Spot *>(spotStorage)->~Spot();

    reinterpret_cast<Dialogs *>(dialogsStorage)->~Dialogs();

    unsigned char dialogStorage[0x10];
    reinterpret_cast<Dialog *>(dialogStorage)->~Dialog();

    reinterpret_cast<GraphicWin *>(dialogsStorage)->~GraphicWin();

    unsigned char miscStorage1[0x10];
    sub_4066c0();
    *reinterpret_cast<void **>(miscStorage1) = g_006693ac;
    *reinterpret_cast<int *>(g_009b3374) = *reinterpret_cast<int *>(miscStorage1 + 4);

    unsigned char miscStorage2[0x10];
    sub_406820();

    unsigned char spriteStorage[0x10];
    reinterpret_cast<Sprite *>(spriteStorage)->close();

    unsigned char flatButtonStorage1[0x10];
    reinterpret_cast<FlatButton *>(flatButtonStorage1)->~FlatButton();

    unsigned char flatButtonStorage2[0x10];
    reinterpret_cast<FlatButton *>(flatButtonStorage2)->~FlatButton();

    unsigned char heapStorage[0x10];
    reinterpret_cast<Heap *>(heapStorage)->shutdown();

    reinterpret_cast<GraphicWin *>(popupStorage)->~GraphicWin();
}
