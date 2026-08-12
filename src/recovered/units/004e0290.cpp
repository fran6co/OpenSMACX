// ORIGINAL: 0x004E0290 FILE
// name      ?editor_reload@Console@@QAEXXZ
// size      1068 bytes
// spans     0x004E0290-0x004E05D9;0x0065C0C8-0x0065C1AB
// prototype void (__thiscall ?editor_reload@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   16
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x00406380 0x004064D0 0x00406880 0x00586050 0x005B0D70 0x005BFAB0 0x005D4DD0 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00612A70 0x00645460 0x00645550
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004E0290
// measured tier  NO_COMPILE
// refusal        u004e0290.cpp(43) : error C2059: syntax error : ';'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e0290/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?editor_reload@Console@@QAEXXZ  at 0x004E0290  (1068 bytes)
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
struct sectionID;
struct srcFileID;

// ---- callees, declared and never defined (a definition would be inlined) ----
class BaseButton { public:
    ~BaseButton();
};
class BasePop { public:
    int exec(int, int (__cdecl *)());
    ~BasePop();
};
class Dialogs { public:
    int item(int8* lpString, int position);
};
class FlatButton { public:
    void close();
    ~FlatButton();
};
class GraphicWin { public:
    ~GraphicWin();
};
class Popup { public:
    Popup();
    void close();
    void start(int8* srcFileID, int8* sectionID, int, int8*, int, GraphicWin*);
};
class Scroll { public:
    void close();
};
extern "C" int _strcpy();  // arity unknown
int __cdecl X_pop_ask(int8*, int, int8*, int (__cdecl *)(), int);
int __cdecl __alloca_probe();
void __cdecl compute_faction_modifiers(int factionID);
void __cdecl read_faction(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0065c1a1 = (int *)0x0065C1A1;
static int *const g_006695c0 = (int *)0x006695C0;
static int *const g_006695c8 = (int *)0x006695C8;
static int *const g_0066974c = (int *)0x0066974C;
static int *const g_00669754 = (int *)0x00669754;
static int *const g_00669d50 = (int *)0x00669D50;
static int *const g_00669d58 = (int *)0x00669D58;
static int *const g_0067ac60 = (int *)0x0067AC60;
static int *const g_006890fc = (int *)0x006890FC;
static int *const g_00689108 = (int *)0x00689108;
static int *const g_006a721c = (int *)0x006A721C;
static int *const g_009472e8 = (int *)0x009472E8;
static int *const g_00949a2c = (int *)0x00949A2C;
static int *const g_009b8aa8 = (int *)0x009B8AA8;
static int *const g_009bb5e8 = (int *)0x009BB5E8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class Console { public:
    void editor_reload();
};

extern "C" char *__cdecl my_strcpy(char *, const char *);
static int *const g_00946a54 = (int *)0x00946A54;

void Console::editor_reload() {
    // Popup is an opaque callee type here (methods only, no agreed size),
    // so this local (stack, non-pointer) instance can't get the original's
    // real frame slot - see the WALL note filed for this address (same
    // class of issue as 0x004589C0/0x00497AC0/0x004055A0, but for Popup).
    Popup popup;
    char *self = reinterpret_cast<char *>(&popup);

    *g_006a721c = 6;
    popup.start(reinterpret_cast<int8 *>(g_009b8aa8), reinterpret_cast<int8 *>(g_006890fc),
                -1, 0, 0x40, 0);

    int position = 1;
    char *record = reinterpret_cast<char *>(g_009472e8);
    do {
        *g_009bbff0 = *reinterpret_cast<int *>(record + 4);
        *g_009bbfec = *reinterpret_cast<int *>(record);
        reinterpret_cast<Dialogs *>(self + 0x21D0)->item(
            reinterpret_cast<int8 *>(record - 0x18), position);
        record += 0x59c;
        ++position;
    } while (record < reinterpret_cast<char *>(g_00949a2c));

    int selected = reinterpret_cast<BasePop *>(self)->exec(0, 0);
    *g_006a721c = 0;

    if (selected < 0) {
        popup.close();
        Scroll scroll1;
        scroll1.close();
        FlatButton fb1;
        fb1.close();
        BaseButton bb1;
    } else {
        char *table_a = reinterpret_cast<char *>(g_00946a54) + selected * 0x59c;
        int ask = X_pop_ask(reinterpret_cast<int8 *>(g_00689108), 0x50,
                             reinterpret_cast<int8 *>(table_a), 0, 0);
        if (ask == 0) {
            my_strcpy(table_a, reinterpret_cast<char *>(g_009bb5e8));
            my_strcpy(table_a + 0x18, reinterpret_cast<char *>(g_009bb5e8));
            read_faction(selected);
            compute_faction_modifiers(selected);
            popup.close();
            Scroll scroll1;
            scroll1.close();
            FlatButton fb1;
            fb1.close();
            BaseButton bb1;
            FlatButton fb2;
            fb2.close();
            BaseButton bb2;
        } else {
            popup.close();
            Scroll scroll1;
            scroll1.close();
            FlatButton fb1;
            fb1.close();
            BaseButton bb1;
        }
    }
}
