// ORIGINAL: 0x0042C650 FILE
// name      ?draw_adv_concept@Datalink@@QAEXXZ
// size      1074 bytes
// spans     0x0042C650-0x0042C9C0;0x00653650-0x00653712
// prototype void (__thiscall ?draw_adv_concept@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   25
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406880 0x00406910 0x005D45B0 0x005D4DD0 0x005D5930 0x005DAC70 0x005DACB0 0x005DD130 0x005E3820 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608980 0x00608E10 0x00629490 0x00629780 0x006453E0 0x00645470 0x00645550 0x0064FC88
// indirect  0x0042C743 0x0042C7DF
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0042C650
// measured tier  NO_COMPILE
// refusal        u0042c650.cpp(45) : error C2059: syntax error : ';' u0042c650.cpp(47) : error C2143: syntax error : missing ';' before 'this' u0042c650.cpp(47) : error C2143: s
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0042c650/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_adv_concept@Datalink@@QAEXXZ  at 0x0042C650  (1074 bytes)
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

struct Font;
struct RECT;
struct __cdecl;
struct int8;
struct this;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BaseButton { public:
    ~BaseButton();
};
class BasePop { public:
    void close();
};
class Buffer { public:
    int set_font(Font*, Font*, Font*, Font*);
    int write_cent_l(int8*, RECT*, int);
    void set_text_color(int, int, int, int);
};
class Dialog { public:
    ~Dialog();
};
class Dialogs { public:
    ~Dialogs();
};
class FlatButton { public:
    void close();
    ~FlatButton();
};
class GraphicWin { public:
    void soft_update(RECT*);
    ~GraphicWin();
};
class Heap { public:
    void shutdown();
};
class Popup { public:
    Popup();
    void close();
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
class StringBox { public:
    void add_fixup();
    void clear();
};
class StringList { public:
    int load(int8*, int8*, int, void (__cdecl *)(int8* this));
};
int __cdecl __alloca_probe();
int __cdecl __itoa();
int __cdecl _strcat();
int __cdecl _strlen();
int __cdecl sub_4066c0();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1
class VCall { public:
    virtual void slot000();
    virtual void slot001();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00653708 = (int *)0x00653708;
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006695c0 = (int *)0x006695C0;
static int *const g_006695c8 = (int *)0x006695C8;
static int *const g_0066974c = (int *)0x0066974C;
static int *const g_00669754 = (int *)0x00669754;
static int *const g_006698cc = (int *)0x006698CC;
static int *const g_006698d4 = (int *)0x006698D4;
static int *const g_00669d50 = (int *)0x00669D50;
static int *const g_00669d58 = (int *)0x00669D58;
static int *const g_00673e90 = (int *)0x00673E90;
static int *const g_00683354 = (int *)0x00683354;
static int *const g_00683360 = (int *)0x00683360;
static int *const g_009b3374 = (int *)0x009B3374;
static int *const g_009b86a0 = (int *)0x009B86A0;

class Datalink { public:
    void draw_adv_concept();
};

void Datalink::draw_adv_concept() {
// Shim for the unresolved receiver-taking callee (rule 4); used twice here,
// each on its own embedded slot.
struct Shim4066c0 { void teardown(); };

// Second shim: the emitter's VCall only declares nullary slots, but the
// disassembly pushes one argument before the slot-1 dispatch (rule 4).
class VCallArg { public:
    virtual void slot000();
    virtual void slot001(int);
};

extern "C" int __cdecl _strcat(char *, const char *);
extern "C" int __cdecl _strlen(const char *);
extern "C" int __cdecl __itoa(int, char *, int);
extern "C" void (__stdcall *raw_0066931c)(char *);

void Datalink::draw_adv_concept() {
    char *self = reinterpret_cast<char *>(this);
    Popup popup;
    char *base = reinterpret_cast<char *>(&popup);

    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    int concept_id = *reinterpret_cast<int *>(self + 0x29e0);
    buf->set_font(reinterpret_cast<Font *>(self + 0x1017c), 0, 0, 0);
    buf->set_text_color(0xe0, -1, 1, 1);

    char *msg = reinterpret_cast<char *>(g_009b86a0);
    *msg = 0;

    int concept_key = concept_id + 0x2710;
    int *p = *reinterpret_cast<int **>(self + 0xf628);
    int off = *reinterpret_cast<int *>(reinterpret_cast<char *>(p) + 8);
    int node = *reinterpret_cast<int *>(self + off + 0xf6ec);
    char *table = self + off + 0xf6e4;
    if (node != 0) {
        int count = *reinterpret_cast<int *>(table + 0x10);
        *reinterpret_cast<int *>(table + 0x14) = 0;
        *reinterpret_cast<int *>(table + 0xc) = node;
        if (count > 0) {
            int i = 0;
            do {
                int cur = *reinterpret_cast<int *>(table + 0xc);
                if (*reinterpret_cast<int *>(cur + 4) == concept_key) break;
                (*reinterpret_cast<int *>(table + 0x14))++;
                cur = *reinterpret_cast<int *>(cur + 0xc);
                i++;
                *reinterpret_cast<int *>(table + 0xc) = cur;
            } while (i < count);
        }
    }

    p = *reinterpret_cast<int **>(self + 0xf628);
    off = *reinterpret_cast<int *>(reinterpret_cast<char *>(p) + 8);
    node = *reinterpret_cast<int *>(self + off + 0xf6ec);
    table = self + off + 0xf6e4;
    char *name = 0;
    if (node != 0) {
        int cur = *reinterpret_cast<int *>(table + 0xc);
        cur = *reinterpret_cast<int *>(cur + 8);
        name = *reinterpret_cast<char **>(cur + 4);
    }
    _strcat(msg, name);

    raw_0066931c(msg);
    char *edx_msg = reinterpret_cast<char *>(g_009b86a0);
    if (edx_msg != 0) {
        int len = _strlen(edx_msg);
        char *stringbox_ptr = self + 0x10514;
        (void)stringbox_ptr;
        RECT *rect = reinterpret_cast<RECT *>(self + 0x10364);
        buf->write_cent_l(reinterpret_cast<int8 *>(msg), rect, len);
    }

    *msg = 0;
    _strcat(msg, reinterpret_cast<char *>(g_00683354));

    int itoa_val = *reinterpret_cast<int *>(self + 0x29e0);
    char itoa_buf[0x64];
    __itoa(itoa_val, itoa_buf, 0xa);
    _strcat(msg, itoa_buf);

    StringBox *sbox = reinterpret_cast<StringBox *>(self + 0x10514);
    sbox->clear();
    reinterpret_cast<StringList *>(self + 0x13084)
        ->load(reinterpret_cast<int8 *>(g_00683360), reinterpret_cast<int8 *>(msg), 1, 0);
    sbox->add_fixup();
    reinterpret_cast<VCallArg *>(sbox)->slot001(0);

    reinterpret_cast<GraphicWin *>(this)->soft_update(
        reinterpret_cast<RECT *>(self + 0x10384));

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
    int saved1 = *reinterpret_cast<int *>(shim1 + 4);
    *reinterpret_cast<int *>(shim1) = 0x6693AC;
    *g_009b3374 = saved1;

    char *shim2 = base + 0x2178;
    reinterpret_cast<Shim4066c0 *>(shim2)->teardown();
    int saved2 = *reinterpret_cast<int *>(shim2 + 4);
    *reinterpret_cast<int *>(shim2) = 0x6693AC;
    *g_009b3374 = saved2;

    reinterpret_cast<Sprite *>(base + 0x2118)->close();
    reinterpret_cast<FlatButton *>(base + 0x15A8)->~FlatButton();
    reinterpret_cast<FlatButton *>(base + 0x0A5C)->~FlatButton();
    reinterpret_cast<Heap *>(base + 0x0A28)->shutdown();

    reinterpret_cast<GraphicWin *>(base)->~GraphicWin();
}
