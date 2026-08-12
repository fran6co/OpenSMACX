// ORIGINAL: 0x0042C2E0 FILE
// name      ?draw_concept@Datalink@@QAEXXZ
// size      1071 bytes
// spans     0x0042C2E0-0x0042C64D;0x0065358E-0x00653650
// prototype void (__thiscall ?draw_concept@Datalink@@QAEXXZ)(Datalink* this)
// callers   1   call targets   25
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406880 0x00406910 0x005D45B0 0x005D4DD0 0x005D5930 0x005DAC70 0x005DACB0 0x005DD130 0x005E3820 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608980 0x00608E10 0x00629490 0x00629780 0x006453E0 0x00645470 0x00645550 0x0064FC88
// indirect  0x0042C3D3 0x0042C46C
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0042C2E0
// measured tier  NO_COMPILE
// refusal        u0042c2e0.cpp(206) : error C2065: 'strcat' : undeclared identifier u0042c2e0.cpp(212) : error C2065: 'strlen' : undeclared identifier
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0042c2e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_concept@Datalink@@QAEXXZ  at 0x0042C2E0  (1071 bytes)
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
// `char`, NOT `signed char` - distinct MSVC types that mangle differently
// (PAD vs PAC); the catalogue's `int8` means the former. The on-disk
// scaffold had `struct int8;` (an incomplete-type forward decl that cannot
// satisfy pointer arithmetic below) plus two outright illegal declarations,
// `struct __cdecl;` and `struct this;` - both reserved words, so both are
// hard syntax errors under any compiler. Replaced with the typedef a fresh
// brief emits.
typedef char int8;

struct Font;
struct RECT;

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
    int load(int8*, int8*, int, void (__cdecl *)(int8*));
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
static int *const g_00653646 = (int *)0x00653646;
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
static int *const g_00673de8 = (int *)0x00673DE8;
static int *const g_00683340 = (int *)0x00683340;
static int *const g_00683348 = (int *)0x00683348;
static int *const g_009b3374 = (int *)0x009B3374;
static int *const g_009b86a0 = (int *)0x009B86A0;

class Datalink { public:
    void draw_concept();
};

// itoa's real 3-argument CRT shape isn't the catalogued `_itoa()`'s nullary
// one; called through its address directly instead, matching how address
// 0x00588460 handled the analogous BasePop callees.
extern "C" char *__cdecl fn_0064fc88(int, char *, int);

void Datalink::draw_concept() {
    char *self = reinterpret_cast<char *>(this);

    // `Popup` is a large local aggregate the context leaves opaque; a raw
    // byte buffer stands in and every nested widget is reached by the byte
    // offset the disassembly shows relative to its start (same technique as
    // 0x0047EC60/NetWin::pick_ocean).
    char popup_storage[0x53E0];
    char *popup = popup_storage;
    *reinterpret_cast<Popup *>(popup) = Popup();

    int32_t id = *reinterpret_cast<int32_t *>(self + 0x29e0);
    int32_t ebx = 0;

    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    buf->set_font(reinterpret_cast<Font *>(self + 0x1017c), 0, 0, 0);
    buf->set_text_color(0xe0, -1, 1, 1);

    *reinterpret_cast<char *>(g_009b86a0) = 0;

    char *edx1 = *reinterpret_cast<char **>(self + 0xf628);
    char *eax1 = *reinterpret_cast<char **>(edx1 + 8);
    int32_t count = *reinterpret_cast<int32_t *>(eax1 + reinterpret_cast<int32_t>(self) + 0xf6ec);
    char *rec = eax1 + reinterpret_cast<int32_t>(self) + 0xf6e4;
    if (count != 0) {
        int32_t limit = *reinterpret_cast<int32_t *>(rec + 0x10);
        int32_t n = 0;
        *reinterpret_cast<int32_t *>(rec + 0x14) = 0;
        *reinterpret_cast<int32_t *>(rec + 0xc) = count;
        if (limit > 0) {
            do {
                int32_t cur = *reinterpret_cast<int32_t *>(rec + 0xc);
                if (*reinterpret_cast<int32_t *>(cur + 4) == id) {
                    ebx = 0;
                    break;
                }
                int32_t hits = *reinterpret_cast<int32_t *>(rec + 0x14);
                hits++;
                n++;
                *reinterpret_cast<int32_t *>(rec + 0x14) = hits;
                *reinterpret_cast<int32_t *>(rec + 0xc) = *reinterpret_cast<int32_t *>(cur + 0xc);
            } while (n < limit);
        }
    }

    char *edx2 = *reinterpret_cast<char **>(self + 0xf628);
    char *eax2 = *reinterpret_cast<char **>(edx2 + 8);
    int32_t count2 = *reinterpret_cast<int32_t *>(eax2 + reinterpret_cast<int32_t>(self) + 0xf6ec);
    char *rec2 = eax2 + reinterpret_cast<int32_t>(self) + 0xf6e4;
    char *name;
    if (count2 == 0) {
        name = 0;
    } else {
        int32_t cur2 = *reinterpret_cast<int32_t *>(rec2 + 0xc);
        int32_t p1 = *reinterpret_cast<int32_t *>(cur2 + 8);
        name = *reinterpret_cast<char **>(p1 + 4);
    }
    strcat(reinterpret_cast<char *>(g_009b86a0), name);

    reinterpret_cast<VCall *>(g_0066931c)->slot001();

    char *msg = reinterpret_cast<char *>(g_009b86a0);
    if (msg) {
        unsigned int len = strlen(msg);
        RECT *rect = reinterpret_cast<RECT *>(self + 0x10364);
        buf->write_cent_l(reinterpret_cast<char *>(g_009b86a0), rect, len);
    }

    *reinterpret_cast<char *>(g_009b86a0) = 0;
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00683340));

    char numbuf[0x1c];
    fn_0064fc88(id, numbuf, 0xa);
    strcat(reinterpret_cast<char *>(g_009b86a0), numbuf);

    StringBox *sbox = reinterpret_cast<StringBox *>(self + 0x10514);
    sbox->clear();
    reinterpret_cast<StringList *>(self + 0x13084)
        ->load(reinterpret_cast<char *>(g_00683348), reinterpret_cast<char *>(g_009b86a0), 1, 0);
    sbox->add_fixup();

    typedef void (__cdecl *SBoxFn)(StringBox *, int32_t);
    SBoxFn fn = *reinterpret_cast<SBoxFn *>(*reinterpret_cast<int32_t *>(sbox) + 4);
    fn(sbox, ebx);

    reinterpret_cast<GraphicWin *>(self)->soft_update(reinterpret_cast<RECT *>(self + 0x10384));

    *reinterpret_cast<int32_t *>(popup) = reinterpret_cast<int32_t>(g_006695c8);
    *reinterpret_cast<int32_t *>(popup + 0x444) = reinterpret_cast<int32_t>(g_006695c0);
    reinterpret_cast<Popup *>(popup)->close();

    char *scroll = popup + 0x3230;
    *reinterpret_cast<int32_t *>(scroll) = reinterpret_cast<int32_t>(g_00669d58);
    *reinterpret_cast<int32_t *>(scroll + 0x444) = reinterpret_cast<int32_t>(g_00669d50);
    reinterpret_cast<Scroll *>(scroll)->close();

    char *fb1 = popup + 0x4828;
    *reinterpret_cast<int32_t *>(fb1) = reinterpret_cast<int32_t>(g_00669754);
    *reinterpret_cast<int32_t *>(fb1 + 0x444) = reinterpret_cast<int32_t>(g_0066974c);
    reinterpret_cast<FlatButton *>(fb1)->close();
    reinterpret_cast<BaseButton *>(fb1)->~BaseButton();

    char *fb2 = popup + 0x3cdc;
    *reinterpret_cast<int32_t *>(fb2) = reinterpret_cast<int32_t>(g_00669754);
    *reinterpret_cast<int32_t *>(fb2 + 0x444) = reinterpret_cast<int32_t>(g_0066974c);
    reinterpret_cast<FlatButton *>(fb2)->close();
    reinterpret_cast<BaseButton *>(fb2)->~BaseButton();

    reinterpret_cast<GraphicWin *>(scroll)->~GraphicWin();

    *reinterpret_cast<int32_t *>(popup) = reinterpret_cast<int32_t>(g_006698d4);
    *reinterpret_cast<int32_t *>(popup + 0x444) = reinterpret_cast<int32_t>(g_006698cc);
    reinterpret_cast<BasePop *>(popup)->close();

    reinterpret_cast<Spot *>(popup + 0x2348)->~Spot();
    reinterpret_cast<Dialogs *>(popup + 0x3088)->~Dialogs();
    reinterpret_cast<Dialog *>(popup + 0x2670)->~Dialog();
    reinterpret_cast<GraphicWin *>(popup + 0x3088)->~GraphicWin();

    sub_4066c0();
    int32_t v1 = *reinterpret_cast<int32_t *>(popup + 0x3234);
    *reinterpret_cast<int32_t *>(popup + 0x3238) = reinterpret_cast<int32_t>(g_006693ac);
    *g_009b3374 = v1;

    sub_4066c0();
    int32_t v2 = *reinterpret_cast<int32_t *>(popup + 0x3264);
    *reinterpret_cast<int32_t *>(popup + 0x3268) = reinterpret_cast<int32_t>(g_006693ac);
    *g_009b3374 = v2;

    reinterpret_cast<Sprite *>(popup + 0x32c8)->close();
    reinterpret_cast<FlatButton *>(popup + 0x3e38)->~FlatButton();
    reinterpret_cast<FlatButton *>(popup + 0x4984)->~FlatButton();
    reinterpret_cast<Heap *>(popup + 0x49b8)->shutdown();
    reinterpret_cast<GraphicWin *>(popup)->~GraphicWin();
}
