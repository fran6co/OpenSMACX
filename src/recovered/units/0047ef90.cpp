// ORIGINAL: 0x0047EF90 FILE
// name      ?pick_erosion@NetWin@@QAEXXZ
// size      1028 bytes
// spans     0x0047EF90-0x0047F2BC;0x006571D5-0x006572AD
// prototype void (__thiscall ?pick_erosion@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   26
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406820 0x00406880 0x00406910 0x004E25E0 0x0059D250 0x0059D3A0 0x005D45B0 0x005D4DD0 0x005E3820 0x005EC8A0 0x005ED240 0x005F8800 0x005F88A0 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00630080 0x00645550
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0047EF90
// measured tier  NO_COMPILE
// refusal        u0047ef90.cpp(43) : error C2059: syntax error : ';'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0047ef90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pick_erosion@NetWin@@QAEXXZ  at 0x0047EF90  (1028 bytes)
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
static int *const g_006572a3 = (int *)0x006572A3;
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
static int *const g_00676d58 = (int *)0x00676D58;
static int *const g_0090e8e0 = (int *)0x0090E8E0;
static int *const g_0090e8e8 = (int *)0x0090E8E8;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f0 = (int *)0x0093D4F0;
static int *const g_0093d4f4 = (int *)0x0093D4F4;
static int *const g_009b3374 = (int *)0x009B3374;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class NetWin { public:
    void pick_erosion();
};

// Shims for two unresolved receiver-taking callees the catalogue's
// fallback declares nullary __cdecl free functions for (rule 4).
struct Shim4066c0 { void teardown(); };
struct Shim406820 { void teardown(); };

extern "C" void *memcpy(void *, const void *, unsigned int);

void NetWin::pick_erosion() {
    Popup popup;
    char *base = reinterpret_cast<char *>(&popup);
    // Two-vtable-poke idiom (offsets 0x444 apart) re-views the freshly
    // constructed GraphicWin-derived object as this screen's own popup type.
    *reinterpret_cast<int *>(base) = 0x66A8A8;
    *reinterpret_cast<int *>(base + 0x444) = 0x66A8A0;

    {
        PullDown pulldown;
        char *pd = reinterpret_cast<char *>(&pulldown);
        *reinterpret_cast<int *>(pd) = 0x66A738;
        *reinterpret_cast<int *>(pd + 0x444) = 0x66A730;

        reinterpret_cast<PopMenu *>(base)->init();

        Dialogs *dlg = reinterpret_cast<Dialogs *>(base + 0x21D0);
        int *table = *reinterpret_cast<int **>(g_009b90f8);
        int title = reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(reinterpret_cast<char *>(table) + 0xfe4));
        dlg->item(reinterpret_cast<int8 *>(title), 0);

        int idx = 0;
        int off = 0xf6c;
        do {
            table = *reinterpret_cast<int **>(g_009b90f8);
            int value = *reinterpret_cast<int *>(reinterpret_cast<char *>(table) + off);
            int entry = reinterpret_cast<Strings *>(g_009b90d8)->get(value);
            idx++;
            dlg->item(reinterpret_cast<int8 *>(entry), idx);
            off += 4;
        } while (off < 0xf78);

        int mouse_a, mouse_b;
        reinterpret_cast<Win *>(this)->get_mouse_pos(&mouse_a, &mouse_b);
        reinterpret_cast<Win *>(this)->client_to_screen(&mouse_a, &mouse_b);
        reinterpret_cast<PopMenu *>(base)->exec(
            mouse_a, mouse_b, reinterpret_cast<int (__cdecl *)()>(g_00483240));

        reinterpret_cast<AlphaNet *>(g_0093cd90)->pid_2_idx(*g_0093d4f0);

        short send_buf[8];
        int copy_buf[6];
        memcpy(copy_buf, reinterpret_cast<void *>(g_0090e8e0), sizeof(copy_buf));
        send_buf[0] = 0x2f02;
        reinterpret_cast<Net *>(g_0093cd90)->send(send_buf, 0x28, *g_0093d4f4, 1);
    } // pulldown destructs here (matches the mid-function ~PullDown call)

    *reinterpret_cast<int *>(base) = 0x6695C8;
    *reinterpret_cast<int *>(base + 0x444) = 0x6695C0;
    popup.close();

    char *scroll = base + 0x3230;
    *reinterpret_cast<int *>(scroll) = 0x669D58;
    *reinterpret_cast<int *>(scroll + 0x444) = 0x669D50;
    reinterpret_cast<Scroll *>(scroll)->close();

    char *fb1 = base + 0x4828;
    *reinterpret_cast<int *>(fb1) = 0x669754;
    *reinterpret_cast<int *>(fb1 + 0x444) = 0x66974C;
    reinterpret_cast<FlatButton *>(fb1)->close();
    reinterpret_cast<BaseButton *>(fb1)->~BaseButton();

    char *fb2 = base + 0x3CDC;
    *reinterpret_cast<int *>(fb2) = 0x669754;
    *reinterpret_cast<int *>(fb2 + 0x444) = 0x66974C;
    reinterpret_cast<FlatButton *>(fb2)->close();
    reinterpret_cast<BaseButton *>(fb2)->~BaseButton();

    reinterpret_cast<GraphicWin *>(scroll)->~GraphicWin();

    *reinterpret_cast<int *>(base) = 0x6698D4;
    *reinterpret_cast<int *>(base + 0x444) = 0x6698CC;
    reinterpret_cast<BasePop *>(base)->close();

    reinterpret_cast<Spot *>(base + 0x3098)->~Spot();
    reinterpret_cast<Dialogs *>(base + 0x2358)->~Dialogs();
    reinterpret_cast<Dialog *>(base + 0x2D70)->~Dialog();
    reinterpret_cast<GraphicWin *>(base + 0x2358)->~GraphicWin();

    char *shim1 = base + 0x21A8;
    reinterpret_cast<Shim4066c0 *>(shim1)->teardown();
    int saved = *reinterpret_cast<int *>(shim1 + 4);
    *reinterpret_cast<int *>(shim1) = 0x6693AC;
    *g_009b3374 = saved;

    reinterpret_cast<Shim406820 *>(base + 0x2150)->teardown();
    reinterpret_cast<Sprite *>(base + 0x2118)->close();
    reinterpret_cast<FlatButton *>(base + 0x15A8)->~FlatButton();
    reinterpret_cast<FlatButton *>(base + 0x0A5C)->~FlatButton();
    reinterpret_cast<Heap *>(base + 0x0A28)->shutdown();

    reinterpret_cast<GraphicWin *>(base)->~GraphicWin();
}
