// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0047E930
// name           ?pick_planet_size@NetWin@@QAEXXZ
// size           1032 bytes
// measured tier  NO_COMPILE
// refusal        u0047e930.cpp(43) : error C2059: syntax error : ';' u0047e930.cpp(168) : warning C4291: 'void *__cdecl operator new(unsigned int,void *)' : no matching operator
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0047e930/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pick_planet_size@NetWin@@QAEXXZ  at 0x0047E930  (1032 bytes)
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
static int *const g_006570f3 = (int *)0x006570F3;
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
static int *const g_00676be8 = (int *)0x00676BE8;
static int *const g_0080a6f0 = (int *)0x0080A6F0;
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_0090e8e6 = (int *)0x0090E8E6;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_009b3374 = (int *)0x009B3374;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class NetWin { public:
    void pick_planet_size();
};

// This function builds a large multi-object popup-menu dialog (Popup +
// PopMenu + PullDown + Dialogs + Scroll + two FlatButtons + BaseButtons +
// GraphicWin bases + Spot + Dialog + Sprite + Heap, ~0x6300 bytes of locals)
// and is compiled with full C++ SEH: a per-object EH state variable at
// [ebp-4], a frame handler at 0x006570F3 and one funclet per partially
// constructed prefix (0x00657025-0x006570F3). That table is generated
// mechanically from the REAL sizeof/layout of each local and the order they
// go out of scope - not from anything expressible with opaque, zero-member
// shells. Faithful call sequence, not a byte-exact frame: WALL, see report.
extern "C" void *memcpy(void *, const void *, unsigned int);

// PopMenu::exec() is catalogued void (QAEXHHP6AHXZ@Z) but the disasm tests
// eax right after the call. A differently named opaque shell reaches the
// same vtable-free thiscall slot; the call relocation is masked by the
// comparison regardless of the symbol it names.
class PopMenuRow { public:
    int exec(int, int, int (__cdecl *)());
};

inline void *operator new(unsigned int, void *place) { return place; }

void NetWin::pick_planet_size() {
    char *self = reinterpret_cast<char *>(this);

    unsigned char popupStorage[0x450];
    Popup *popup = new (popupStorage) Popup();
    *reinterpret_cast<void **>(popupStorage) =
        reinterpret_cast<void *>(g_0066a8a8);
    *reinterpret_cast<void **>(popupStorage + 0x444) =
        reinterpret_cast<void *>(g_0066a8a0);

    unsigned char pullDownStorage[0x460];
    PullDown *pulldown = new (pullDownStorage) PullDown();
    *reinterpret_cast<void **>(pullDownStorage) =
        reinterpret_cast<void *>(g_0066a738);
    *reinterpret_cast<void **>(pullDownStorage + 0x444) =
        reinterpret_cast<void *>(g_0066a730);

    reinterpret_cast<PopMenu *>(popupStorage)->init();

    int *strTable = *reinterpret_cast<int **>(g_009b90f8);
    int titleId = *reinterpret_cast<int *>(
        reinterpret_cast<char *>(strTable) + 0xfe4);
    char *title = reinterpret_cast<char *>(
        reinterpret_cast<Strings *>(g_009b90d8)->get(titleId));

    unsigned char dialogsStorage[0x2200];
    Dialogs *dialogs = new (dialogsStorage) Dialogs();
    dialogs->item(reinterpret_cast<int8 *>(title), 0);

    int count = *g_0080a6f0;
    char *entry = self + 0x6614;
    for (int i = 1; i <= count; i++) {
        dialogs->item(reinterpret_cast<int8 *>(entry), i);
        entry += 0x80;
    }

    int mouseX, mouseY;
    reinterpret_cast<Win *>(self)->get_mouse_pos(&mouseX, &mouseY);
    reinterpret_cast<Win *>(self)->client_to_screen(&mouseX, &mouseY);

    int result = reinterpret_cast<PopMenuRow *>(popupStorage)->exec(
        mouseX, mouseY, reinterpret_cast<int (__cdecl *)()>(g_00483240));
    if (result >= 0) {
        *reinterpret_cast<unsigned char *>(g_0090e8e6) =
            static_cast<unsigned char>(result - 1);
    }

    unsigned int pid = *reinterpret_cast<unsigned int *>(g_0093d4f0);
    reinterpret_cast<AlphaNet *>(g_0093cd90)->pid_2_idx(pid);
    unsigned int arg3 = *reinterpret_cast<unsigned int *>(g_0093d4f4);

    unsigned char msgBuf[0x28];
    memcpy(msgBuf + 0x10, g_0090e8e0, 6 * sizeof(int));
    *reinterpret_cast<unsigned short *>(msgBuf) = 0x2f02;
    reinterpret_cast<Net *>(g_0093cd90)->send(msgBuf, 0x28, arg3, 1);

    pulldown->~PullDown();
    popup->close();

    unsigned char scrollStorage[0x30d4];
    *reinterpret_cast<void **>(scrollStorage) =
        reinterpret_cast<void *>(g_006695c8);
    *reinterpret_cast<void **>(scrollStorage + 0x444) =
        reinterpret_cast<void *>(g_006695c0);
    reinterpret_cast<Scroll *>(scrollStorage)->close();

    unsigned char flatButton1Storage[0x1ae0];
    *reinterpret_cast<void **>(flatButton1Storage) =
        reinterpret_cast<void *>(g_00669754);
    *reinterpret_cast<void **>(flatButton1Storage + 0x444) =
        reinterpret_cast<void *>(g_0066974c);
    reinterpret_cast<FlatButton *>(flatButton1Storage)->close();
    reinterpret_cast<BaseButton *>(flatButton1Storage)->~BaseButton();

    unsigned char flatButton2Storage[0x2630];
    *reinterpret_cast<void **>(flatButton2Storage) =
        reinterpret_cast<void *>(g_00669754);
    *reinterpret_cast<void **>(flatButton2Storage + 0x444) =
        reinterpret_cast<void *>(g_0066974c);
    reinterpret_cast<FlatButton *>(flatButton2Storage)->close();
    reinterpret_cast<BaseButton *>(flatButton2Storage)->~BaseButton();

    reinterpret_cast<GraphicWin *>(scrollStorage)->~GraphicWin();

    *reinterpret_cast<void **>(popupStorage) =
        reinterpret_cast<void *>(g_006698d4);
    *reinterpret_cast<void **>(popupStorage + 0x444) =
        reinterpret_cast<void *>(g_006698cc);
    reinterpret_cast<BasePop *>(popupStorage)->close();

    unsigned char spotStorage[0x3270];
    reinterpret_cast<Spot *>(spotStorage)->~Spot();

    unsigned char dialogStorage[0x330];
    reinterpret_cast<Dialog *>(dialogStorage)->~Dialog();

    dialogs->~Dialogs();
    reinterpret_cast<GraphicWin *>(dialogsStorage)->~GraphicWin();

    sub_4066c0();
    int savedField = *reinterpret_cast<int *>(popupStorage + 4);
    *reinterpret_cast<int *>(g_009b3374) = savedField;

    sub_406820();

    unsigned char spriteStorage[0x38];
    reinterpret_cast<Sprite *>(spriteStorage)->close();

    unsigned char flatButton3Storage[0xb50];
    reinterpret_cast<FlatButton *>(flatButton3Storage)->~FlatButton();

    unsigned char flatButton4Storage[0x34];
    reinterpret_cast<FlatButton *>(flatButton4Storage)->~FlatButton();

    reinterpret_cast<Heap *>(popupStorage)->shutdown();
    reinterpret_cast<GraphicWin *>(popupStorage)->~GraphicWin();
}
