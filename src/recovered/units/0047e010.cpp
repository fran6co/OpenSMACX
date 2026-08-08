// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0047E010
// name           ?pick_num_players@NetWin@@QAEXXZ
// size           1027 bytes
// measured tier  NO_COMPILE
// refusal        u0047e010.cpp(43) : error C2059: syntax error : ';' u0047e010.cpp(157) : warning C4291: 'void *__cdecl operator new(unsigned int,void *)' : no matching operator
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0047e010/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pick_num_players@NetWin@@QAEXXZ  at 0x0047E010  (1027 bytes)
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
class Win { public:
    void client_to_screen(int*, int*);
    void get_mouse_pos(int*, int*);
};
int __cdecl __alloca_probe();
int __cdecl __itoa();
int __cdecl _strcat();
int __cdecl sub_406820();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00483240 = (int *)0x00483240;
static int *const g_00656e6b = (int *)0x00656E6B;
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
static int *const g_006769c0 = (int *)0x006769C0;
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_0090e8e1 = (int *)0x0090E8E1;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_0093d4f8 = (int *)0x0093D4F8;
static int *const g_009b86a0 = (int *)0x009B86A0;

class NetWin { public:
    void pick_num_players();
};

// Same SEH-frame situation as ?pick_time@NetWin@@QAEXXZ (0x0047E640):
// this subject builds a stack of local UI objects and relies on the
// compiler's own frame-based unwind (__except_handler3, funclets at
// 0x00656D9D-0x00656E70) to tear them down on an exception. The
// catalogue's scaffold gives these types as opaque shells with named
// cleanup methods rather than real RAII types, so plain C++ here cannot
// make the compiler emit that frame. This reproduces the NORMAL call
// sequence - object construction, the player-count string list, the
// PopMenu init/exec, the net-send bookkeeping, and the teardown chain -
// in the order and with the arguments the disassembly shows.

inline void *operator new(unsigned int, void *p) { return p; }
typedef char *(__cdecl *ItoaFn)(int, char *, int);

void NetWin::pick_num_players() {
    unsigned char frame[0x6328 + 0x20];
    unsigned char *base = frame + 0x6328;
#define AT(off) (base - (off))

    new (AT(0x6328)) Popup();
    new (AT(0xfac)) PullDown();

    *(void **)AT(0x6328) = (void *)g_0066a8a8;
    *(void **)AT(0x5ee4) = (void *)g_0066a8a0;
    *(void **)AT(0xfac) = (void *)g_0066a738;
    *(void **)AT(0xb68) = (void *)g_0066a730;

    ((PopMenu *)AT(0x6328))->init();

    int minPlayers = *(int *)g_0093d4f8;
    if (minPlayers <= 2) {
        minPlayers = 2;
    }
    if (minPlayers < 8) {
        for (int n = minPlayers; n < 8; ++n) {
            char numBuf[0x20];
            *(char *)g_009b86a0 = 0;
            ((ItoaFn)_itoa)(n, numBuf, 10);
            strcat((char *)g_009b86a0, numBuf);
            ((Dialogs *)AT(0x4158))->item((char *)g_009b86a0, n);
        }
    }

    int mouseX, mouseY;
    ((Win *)this)->get_mouse_pos(&mouseX, &mouseY);
    ((Win *)this)->client_to_screen(&mouseX, &mouseY);
    ((PopMenu *)AT(0x6328))->exec(mouseX, mouseY, (int(__cdecl *)())g_00483240);

    ((AlphaNet *)g_0093cd90)->pid_2_idx(*(unsigned long *)g_0093d4f0);

    unsigned char netMsg[0x28];
    *(short *)netMsg = 0x2f02;
    for (int i = 0; i < 6; ++i) {
        ((int *)(netMsg + 0x10))[i] = ((int *)g_0090e8e0)[i];
    }
    ((Net *)g_0093cd90)->send(netMsg, 0x28, *(unsigned long *)g_0093d4f4, 1);

    ((PullDown *)AT(0xfac))->~PullDown();

    *(void **)AT(0x6328) = (void *)g_006695c8;
    *(void **)AT(0x5ee4) = (void *)g_006695c0;
    ((Popup *)AT(0x6328))->close();

    *(void **)AT(0x30f8) = (void *)g_00669d58;
    *(void **)AT(0x2cb4) = (void *)g_00669d50;
    ((Scroll *)AT(0x30f8))->close();

    *(void **)AT(0x1b00) = (void *)g_00669754;
    *(void **)AT(0x16bc) = (void *)g_0066974c;
    ((FlatButton *)AT(0x1b00))->close();
    ((BaseButton *)AT(0x1b00))->~BaseButton();

    *(void **)AT(0x264c) = (void *)g_00669754;
    *(void **)AT(0x2208) = (void *)g_0066974c;
    ((FlatButton *)AT(0x264c))->close();
    ((BaseButton *)AT(0x264c))->~BaseButton();

    ((GraphicWin *)AT(0x30f8))->~GraphicWin();

    *(void **)AT(0x6328) = (void *)g_006698d4;
    *(void **)AT(0x5ee4) = (void *)g_006698cc;
    ((BasePop *)AT(0x6328))->close();

    ((Spot *)AT(0x3290))->~Spot();
    ((Dialogs *)AT(0x3fd0))->~Dialogs();
    ((Dialog *)AT(0x35b8))->~Dialog();
    ((GraphicWin *)AT(0x3fd0))->~GraphicWin();

    sub_406820();
    sub_406820();

    ((Sprite *)AT(0x4210))->close();
    ((FlatButton *)AT(0x4d80))->~FlatButton();
    ((FlatButton *)AT(0x58cc))->~FlatButton();
    ((Heap *)AT(0x5900))->shutdown();
    ((GraphicWin *)AT(0x6328))->~GraphicWin();

#undef AT

}
